#include <cassert>

#include "BitStream.h"

BitStream::BitStream()
    : m_bigBuffer()
    , m_frontBuffer(0)
    , m_frontBits(0)
    , m_backBuffer(0)
    , m_backBits(0)
{}

void BitStream::Push(unsigned int data, unsigned int bits)
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
            m_bigBuffer.push_back(data & 255);
            data >>= 8;
            bits -= 8;
        }
        else
        {
            m_backBuffer |= (data << m_backBits) & 255;
            data >>= 8 - m_backBits;
            bits -= 8 - m_backBits;
            m_bigBuffer.push_back(m_backBuffer);
            m_backBits = 0;
            m_backBuffer = 0;
        }
    }
}

unsigned int BitStream::Pop(const unsigned int bits)
{
    unsigned int data = 0;
    bool res = TryPop(data, bits);
    assert(res);
    return data;
}

bool BitStream::TryPop(unsigned int & data, unsigned int bits)
{
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
                offset += m_frontBits;
            }
            else
            {

                offset += bits;
            }
        }
        // pop from buffer
        if (bits > 0 && !m_bigBuffer.empty())
        {
            assert(m_frontBits == 0);

        }
        // pop from back
        if (bits > 0)
        {
            assert(m_frontBits == 0);
            assert(m_bigBuffer.empty());
            assert(m_backBits < 8);

        }
        return true;
    }
    else
    {
        return false;
    }
}

void BitStream::FlushFront()
{
    if (m_frontBits > 0)
    {

    }
}

void BitStream::FlushBack()
{
    if (m_backBits > 0)
    {

    }
}

unsigned int BitStream::BitsAvailable() const
{
    return m_bigBuffer.size()*8 + m_frontBits + m_backBits;
}
