#include <algorithm>
#include <cassert>

#include "BitBuffer.h"

namespace
{
    // mask used by TryPop and TryPeek
    static const unsigned int mask[] =
    {
        (1u <<  0) - 1,  (1u <<  1) - 1,  (1u <<  2) - 1,  (1u <<  3) - 1,  (1u <<  4) - 1,  (1u <<  5) - 1,  (1u <<  6) - 1,  (1u <<  7) - 1,
        (1u <<  8) - 1,  (1u <<  9) - 1,  (1u << 10) - 1,  (1u << 11) - 1,  (1u << 12) - 1,  (1u << 13) - 1,  (1u << 14) - 1,  (1u << 15) - 1,
        (1u << 16) - 1,  (1u << 17) - 1,  (1u << 18) - 1,  (1u << 19) - 1,  (1u << 20) - 1,  (1u << 21) - 1,  (1u << 22) - 1,  (1u << 23) - 1,
        (1u << 24) - 1,  (1u << 25) - 1,  (1u << 26) - 1,  (1u << 27) - 1,  (1u << 28) - 1,  (1u << 29) - 1,  (1u << 30) - 1,  (1u << 31) - 1
    };
}

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
    if (m_backBits + bits < 32)
    {
        m_backBuffer |= data << m_backBits;
        m_backBits += bits;
        bits = 0;
    }
    else
    {
        m_backBuffer |= (data << m_backBits);
        m_bigBuffer.emplace_back(m_backBuffer);
        m_backBuffer = data >> (32 - m_backBits);
        m_backBits = bits - (32 - m_backBits);
    }
}

void BitBuffer::Push(const std::vector<unsigned char>& data, unsigned int bits)
{
    assert(data.size() == (bits + 7) / 8);
    for (unsigned int i = 0; i < bits / 8; ++i)
    {
        Push(data[i], 8u);
    }
    bits = bits % 8;
    if (bits != 0)
    {
        Push(data.back(),bits);
    }
}

void BitBuffer::Push(const std::vector<unsigned int>& data, unsigned int bits)
{
    assert(data.size() == (bits + 31) / 32);
    for (unsigned int i = 0; i < bits / 32; ++i)
    {
        Push(data[i], 32u);
    }
    bits = bits % 32;
    if (bits != 0)
    {
        Push(data.back(), bits);
    }
}

void BitBuffer::Push(const BitBuffer& data)
{
    Push(data.m_frontBuffer, data.m_frontBits);
    for (const auto c : data.m_bigBuffer)
    {
        Push(c, 32u);
    }
    Push(data.m_backBuffer, data.m_backBits);
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
    unsigned int offset = 0;
    assert(bits <= sizeof(data) * 8);
    data = 0;
    // pop from front if possible
    assert(m_frontBits < 32);
    if (bits <= m_frontBits)
    {
        data |= m_frontBuffer & mask[bits];
        m_frontBits -= bits;
        m_frontBuffer >>= bits;
        bits = 0;
        return true;
    }
    else if (bits <= BitsAvailable() )
    {
        // pop from front
        if (m_frontBits > 0)
        {
            data |= m_frontBuffer & mask[m_frontBits];
            offset += m_frontBits;
            bits -= m_frontBits;
            m_frontBits = 0;
            m_frontBuffer = 0;
        }
        // pop from buffer if possible
        if (!m_bigBuffer.empty())
        {
            assert(m_frontBits == 0);
            data |= (m_bigBuffer.front() & mask[bits]) << offset;
            m_frontBuffer = m_bigBuffer.front() >> bits;
            m_frontBits = 32 - bits;
            m_bigBuffer.pop_front();
            offset += bits;
            bits = 0;
        }
        // pop from back if needed
        else
        {
            assert(m_frontBits == 0);
            assert(m_bigBuffer.empty());
            assert(m_backBits < 32);
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

unsigned int BitBuffer::Peek(const unsigned int bits)
{
    unsigned int data = 0;
    bool res = TryPeek(data, bits);
    assert(res);
    return data;
}

bool BitBuffer::TryPeek(unsigned int& data, unsigned int bits)
{
    unsigned int offset = 0;
    assert(bits <= sizeof(data) * 8);
    data = 0;
    // peek completely from front if possible
    assert(m_frontBits < 32);
    if (bits <= m_frontBits)
    {
        data |= m_frontBuffer & mask[bits];
        return true;
    }
    else if (bits <= BitsAvailable())
    {
        // peek from front
        if (m_frontBits > 0)
        {
            data |= m_frontBuffer & mask[m_frontBits];
            offset += m_frontBits;
            bits -= m_frontBits;
        }
        // peek from buffer if possible
        if (!m_bigBuffer.empty())
        {
            data |= (m_bigBuffer.front() & mask[bits]) << offset;
        }
        // peek from back if buffer empty
        else
        {
            data |= (m_backBuffer & mask[bits]) << offset;
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
            auto temp = m_frontBuffer | (buffer << m_frontBits);
            m_frontBuffer = buffer >> (32 - m_frontBits);
            buffer = temp;
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
    if (m_backBits%8 > 0)
    {
        Push(0, 8-(m_backBits%8));
    }
}

size_t BitBuffer::BitsAvailable() const
{
    return m_bigBuffer.size()*32 + m_frontBits + m_backBits;
}

bool BitBuffer::HasData() const
{
    return m_frontBits > 0 || m_backBits > 0 || !m_bigBuffer.empty();
}

bool BitBuffer::Empty() const
{
    return !HasData();
}

void BitBuffer::RetrieveFrontInts(std::vector<unsigned int>& outBuffer)
{
    FlushFront();
    if (outBuffer.capacity() < m_bigBuffer.size() + outBuffer.size())
    {
        outBuffer.reserve(m_bigBuffer.size() + outBuffer.size());
    }
    outBuffer.insert(outBuffer.end(), m_bigBuffer.begin(), m_bigBuffer.end());
    m_bigBuffer.clear();
}

void BitBuffer::RetrieveFrontBytes(std::vector<unsigned char>& outBuffer)
{
    FlushFront();
    if (outBuffer.capacity()*4 < m_bigBuffer.size() + outBuffer.size()*4 + m_backBits/8)
    {
        outBuffer.reserve(m_bigBuffer.size() + outBuffer.size() + m_backBits/8);
    }
    for (const auto data : m_bigBuffer)
    {
        outBuffer.emplace_back(static_cast<unsigned char>(data      ));
        outBuffer.emplace_back(static_cast<unsigned char>(data >>  8));
        outBuffer.emplace_back(static_cast<unsigned char>(data >> 16));
        outBuffer.emplace_back(static_cast<unsigned char>(data >> 24));
    }
    m_bigBuffer.clear();
    for (unsigned int i = 0; i < m_backBits / 8; ++i)
    {
        outBuffer.emplace_back(static_cast<unsigned char>(m_backBuffer));
        m_backBuffer >>= 8;
    }
    m_backBits = m_backBits % 8;
}
