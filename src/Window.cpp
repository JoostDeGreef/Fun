#include <algorithm>

#include "BitFiFo.h"
#include "Window.h"

class Attempt
{
public:
    Attempt()
        : m_bits()
        , m_len(0)
    {}

    void Set(unsigned char c)
    {
        m_len = 1;
        m_bits.Clear();
        m_bits.Push(c, 8u);
    }
    void Set(unsigned char esc,unsigned char c)
    {
        m_len = 1;
        m_bits.Clear();
        m_bits.Push(esc, 8u);
        m_bits.Push(c, 8u);
    }

    void Clear()
    {
        m_len = 0;
        m_bits.Clear();
    }

    void Swap(Attempt& other)
    {
        other.m_bits.Swap(m_bits);
        std::swap(other.m_len,m_len);
    }

    BitFiFo m_bits;
    unsigned int m_len;
};

void WindowCompressor::Compress(std::vector<unsigned char>& ioBuffer)
{
    m_buffer.insert(m_buffer.end(), ioBuffer.begin(), ioBuffer.end());
    ioBuffer.clear();
    Attempt attempt;
    Attempt choice;
    auto UpdateChoice = [&attempt, &choice]()
    {
        if (choice.m_bits.Size() * attempt.m_len > attempt.m_bits.Size() * choice.m_len)
        {
            choice.Swap(attempt);
        }
    };
    auto SaveDistLen = [&attempt, &choice, &UpdateChoice](unsigned int dist, unsigned int len)
    {
        //   esc 00 11 20:dist 8:len : len(6-261)
        //   esc 01     4:dist 2:len : len(3-6)
        //   esc 10    10:dist 4:len : len(4-19)
        //   esc 11    16:dist 6:len : len(5-36)
        if (dist < (1 << 4) && len >= 3)
        {
            attempt.Clear();
            attempt.m_len = std::min(len, 6u);
            attempt.m_bits.Push(escape, 8u);
            attempt.m_bits.Push(1, 2u);
            attempt.m_bits.Push(dist, 4u);
            attempt.m_bits.Push(attempt.m_len - 3, 2u);
            UpdateChoice();
        }
        if (dist < (1 << 10) && len >= 4)
        {
            attempt.Clear();
            attempt.m_len = std::min(len, 19u);
            attempt.m_bits.Push(escape, 8u);
            attempt.m_bits.Push(2, 2u);
            attempt.m_bits.Push(dist, 10u);
            attempt.m_bits.Push(attempt.m_len - 4, 4u);
            UpdateChoice();
        }
        if (dist < (1 << 16) && len >= 5)
        {
            attempt.Clear();
            attempt.m_len = std::min(len, 36u);
            attempt.m_bits.Push(escape, 8u);
            attempt.m_bits.Push(3, 2u);
            attempt.m_bits.Push(dist, 16u);
            attempt.m_bits.Push(attempt.m_len - 5, 6u);
            UpdateChoice();
        }
        if (dist < (1 << 20) && len >= 6)
        {
            attempt.Clear();
            attempt.m_len = std::min(len, 261u);
            attempt.m_bits.Push(escape, 8u);
            attempt.m_bits.Push(0, 2u);
            attempt.m_bits.Push(3, 2u);
            attempt.m_bits.Push(dist, 20u);
            attempt.m_bits.Push(attempt.m_len - 6, 8u);
            UpdateChoice();
        }
    };
    while(m_index < m_buffer.size())
    {
        // just write directly
        if (m_buffer[m_index] == escape)
        {
            choice.Set(escape, 0);
        }
        else
        {
            choice.Set(m_buffer[m_index]);
        }
        // try RLE sequence
        unsigned int len = 1;
        for ( ; len + m_index < m_buffer.size() &&
                len <= 16 &&
                m_buffer[m_index + len] == m_buffer[m_index]
              ; )
        {
            ++len;
        }
        len = std::min(len, 16u);
        if (len > 1)
        {
            attempt.Clear();
            attempt.m_bits.Push(escape, 8u);
            attempt.m_bits.Push(0, 2u);
            if (m_buffer[m_index] == escape)
            {
                attempt.m_bits.Push(0, 2u);
                attempt.m_bits.Push(len-1, 4u);
            }
            else
            {
                attempt.m_bits.Push(1, 2u);
                attempt.m_bits.Push(len - 1, 4u);
                attempt.m_bits.Push(m_buffer[m_index], 8u);
            }
            attempt.m_len = len;
            UpdateChoice();
        }
        // find sequence at some distance
        len = std::max(len, 3u);
        for(unsigned int dist = len+1;
            dist < 1 << 20 &&
            dist <= m_index &&
            len<=261; // max window size, max len
            ++dist)
        {
            unsigned int currlen = 0;
            for (; currlen < 261 && currlen + m_index < m_buffer.size(); ++currlen)
            {
                if (m_buffer[m_index + currlen] != m_buffer[m_index - dist + currlen])
                {
                    break;
                }
            }
            if (currlen > len)
            {
                len = currlen;
                SaveDistLen(dist, len);
            }
        }
        // write the best attempt
        m_index += choice.m_len;
        choice.m_bits.Pop(ioBuffer);
    }
}

void WindowCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    Compress(ioBuffer);
    // write EOF
    BitFiFo bits;
    bits.Push(escape, 8u);
    bits.Push(0, 2u);
    bits.Push(2, 2u);
    bits.Push(0, 4u);
    bits.Pop(ioBuffer);
}

void WindowDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
    auto CopySequence = [&](unsigned int dist, unsigned int len)
    {
        auto index = m_buffer.size();
        for (unsigned int i = 0; i < len; ++i)
        {
            m_buffer.push_back(m_buffer[index-dist+i]);
        }
    };
    auto ReadSequence = [&]()
    {
        if (!m_input.Empty() && !m_eof)
        {
            if (!m_escaped)
            {
                auto c = (unsigned char)m_input.Pop(8u);
                if (c == escape)
                {
                    m_escaped = true;
                }
                else
                {
                    m_buffer.emplace_back(c);
                }
                return true;
            }
            else
            {
                auto code = m_input.Peek(4u);
                switch (code & 3)
                {
                case 0:
                    switch (code >> 2)
                    {
                    case 0: //   esc 00 00  4:len        : (len+1)*esc
                        {
                            code = m_input.Pop(4u);
                            assert(code == 0);
                            auto len = m_input.Pop(4u);
                            for (unsigned int i = 0; i <= len; ++i)
                            {
                                m_buffer.emplace_back(escape);
                            }
                            m_escaped = false;
                            return true;
                        }
                    case 1: //   esc 00 01  4:len 8:val  : (len+1)*val
                        if (m_input.Size() > 8 * 2)
                        {
                            code = m_input.Pop(4u);
                            assert(code == 4);
                            auto len = m_input.Pop(4u);
                            auto val = (unsigned char)m_input.Pop(8u);
                            for (unsigned int i = 0; i <= len; ++i)
                            {
                                m_buffer.emplace_back(val);
                            }
                            m_escaped = false;
                            return true;
                        }
                        break;
                    case 2: //   esc 00 10               : EOF
                        code = m_input.Pop(8u);
                        assert(code == 8);
                        m_eof = true;
                        m_escaped = false;
                        return true;
                    case 3: //   esc 00 11 20:dist 8:len : len(6-261)
                        if (m_input.Size() > 8 * 4)
                        {
                            code = m_input.Pop(4u);
                            assert(code == 12);
                            auto dist = m_input.Pop(20u);
                            unsigned int len = m_input.Pop(8u) + 6;
                            CopySequence(dist, len);
                            m_escaped = false;
                            return true;
                        }
                        break;
                    }
                    break;
                case 1: //   esc 01     4:dist 2:len : len(3-6)
                    {
                        m_input.Pop(2u);
                        auto dist = m_input.Pop(4u);
                        auto len = m_input.Pop(2u) + 3;
                        CopySequence(dist, len);
                        m_escaped = false;
                        return true;
                    }
                    break;
                case 2: //   esc 10    10:dist 4:len : len(4-19)
                    if (m_input.Size() > 8 * 2)
                    {
                        m_input.Pop(2u);
                        auto dist = m_input.Pop(10u);
                        auto len = m_input.Pop(4u) + 4;
                        CopySequence(dist, len);
                        m_escaped = false;
                        return true;
                    }
                    break;
                case 3: //   esc 11    16:dist 6:len : len(5-36)
                    if (m_input.Size() > 8 * 3)
                    {
                        m_input.Pop(2u);
                        auto dist = m_input.Pop(16u);
                        auto len = m_input.Pop(6u) + 5;
                        CopySequence(dist, len);
                        m_escaped = false;
                        return true;
                    }
                    break;
                }
            }
        }
        return false;
    };
    m_input.Push(ioBuffer,8*ioBuffer.size());
    ioBuffer.clear();
    assert(m_input.Size() % 8 == 0);
    while (ReadSequence())
    {
    }
    assert(m_input.Size() % 8 == 0);
    // copy overflow from m_buffer
    // todo
}

void WindowDeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
    if (!m_eof)
    {
        throw std::runtime_error("Incomplete data");
    }
    else if(!m_input.Empty())
    {
        throw std::runtime_error("Extra data after EOF");
    }
    // copy everything from m_buffer
    ioBuffer.insert(ioBuffer.end(), m_buffer.begin(), m_buffer.end());
    m_buffer.clear();
}
