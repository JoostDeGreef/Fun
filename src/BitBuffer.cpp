#include <algorithm>
#include <cassert>

#include "BitBuffer.h"

BitBuffer::BitBuffer()
    : m_bigBuffer()
    , m_frontBuffer(0)
    , m_frontBits(0)
    , m_backBuffer(0)
    , m_backBits(0)
{}

void BitBuffer::Swap(BitBuffer& other)
{
    m_bigBuffer.swap(other.m_bigBuffer);
    std::swap(m_frontBuffer, other.m_frontBuffer);
    std::swap(m_frontBits, other.m_frontBits);
    std::swap(m_backBuffer, other.m_backBuffer);
    std::swap(m_backBits, other.m_backBits);
}

void BitBuffer::Clear()
{
    m_bigBuffer.clear();
    m_frontBuffer = 0;
    m_frontBits = 0;
    m_backBuffer = 0;
    m_backBits = 0;
}

void BitBuffer::Push(unsigned int data, unsigned int bits)
{
    assert(bits <= sizeof(data) * 8);
    assert(m_backBits < 8);
    while (bits > 0)
    {
        if (m_backBits + bits < 8)
        {
            m_backBuffer |= data << m_backBits;
            m_backBits += bits;
            bits = 0;
        }
        else if (m_backBits == 0)
        {
            m_bigBuffer.emplace_back(data & 255);
            data >>= 8;
            bits -= 8;
        }
        else
        {
            m_backBuffer |= (data << m_backBits) & 255;
            data >>= 8 - m_backBits;
            bits -= 8 - m_backBits;
            m_bigBuffer.emplace_back(m_backBuffer);
            m_backBits = 0;
            m_backBuffer = 0;
        }
    }
}

void BitBuffer::Push(const std::vector<unsigned char>& data, unsigned int bits)
{
    assert(data.size() == (bits + 7) / 8);
    for (unsigned int i = 0; i < bits / 8; ++i)
    {
        Push(data[i], 8);
    }
    bits = bits % 8;
    if (bits != 0)
    {
        Push(data.back(),bits);
    }
}


unsigned int BitBuffer::Pop(const unsigned int bits)
{
    unsigned int data = 0;
    bool res = TryPop(data, bits);
    assert(res);
    return data;
}

bool BitBuffer::TryPop(unsigned int & data, unsigned int bits)
{
    static const unsigned int mask[] = {0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383};
    if (bits <= BitsAvailable() )
    {
        unsigned int offset = 0;
        assert(bits <= sizeof(data) * 8);
        data = 0;
        // pop from front
        if (bits > 0 && m_frontBits > 0)
        {
            assert(m_frontBits < 8);
            if (m_frontBits <= bits)
            {
                data |= m_frontBuffer & mask[m_frontBits];
                offset += m_frontBits;
                bits -= m_frontBits;
                m_frontBits = 0;
                m_frontBuffer = 0;
            }
            else
            {
                data |= m_frontBuffer & mask[bits];
                offset += bits;
                m_frontBits -= bits;
                m_frontBuffer >>= bits;
                bits = 0;
            }
        }
        // pop from buffer
        while (bits >= 8 && !m_bigBuffer.empty())
        {
            assert(m_frontBits == 0);
            data |= m_bigBuffer.front() << offset;
            m_bigBuffer.pop_front();
            offset += 8;
            bits -= 8;
        }
        if (bits > 0 && !m_bigBuffer.empty())
        {
            assert(m_frontBits == 0);
            data |= (m_bigBuffer.front() & mask[bits]) << offset;
            m_frontBuffer = m_bigBuffer.front() >> bits;
            m_frontBits = 8 - bits;
            m_bigBuffer.pop_front();
            offset += bits;
            bits = 0;
        }
        // pop from back
        if (bits > 0)
        {
            assert(m_frontBits == 0);
            assert(m_bigBuffer.empty());
            assert(m_backBits < 8);
            data |= (m_backBuffer & mask[bits]) << offset;
            offset += bits;
            m_backBits -= bits;
            m_backBuffer >>= bits;
            bits = 0;
        }
        return true;
    }
    else
    {
        return false;
    }
}

void BitBuffer::FlushFront()
{
    if (m_frontBits > 0)
    {
        unsigned int backBits = m_backBits;
        unsigned int backBuffer = m_backBuffer;
        for (auto& buffer : m_bigBuffer)
        {
            m_frontBuffer |= buffer << m_frontBits;
            buffer = m_frontBuffer & 255;
            m_frontBuffer >>= 8;
        }
        m_backBits = m_frontBits;
        m_backBuffer = m_frontBuffer;
        m_frontBits = 0;
        m_frontBuffer = 0;
        Push(backBuffer, backBits);
    }
}

void BitBuffer::FlushBack()
{
    if (m_backBits > 0)
    {
        Push(0, 8-m_backBits);
    }
}

size_t BitBuffer::BitsAvailable() const
{
    return m_bigBuffer.size()*8 + m_frontBits + m_backBits;
}

bool BitBuffer::HasData() const
{
    return m_frontBits > 0 || m_backBits > 0 || !m_bigBuffer.empty();
}

bool BitBuffer::Empty() const
{
    return !HasData();
}


void BitBuffer::RetrieveFrontBytes(std::vector<unsigned char>& outBuffer)
{
    FlushFront();
    if (outBuffer.capacity() < m_bigBuffer.size() + outBuffer.size())
    {
        outBuffer.reserve(m_bigBuffer.size() + outBuffer.size());
    }
    outBuffer.insert(outBuffer.end(), m_bigBuffer.begin(), m_bigBuffer.end());
    m_bigBuffer.clear();
}
