#include <algorithm>
#include <array>
#include <map>
#include <iterator>

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
    m_window.insert(m_window.end(), ioBuffer.begin(), ioBuffer.end());
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
        //   esc 00    10:dist 4:len : dist(4..3 + 1<<10), len(4..3 + 1<<4)
        //   esc 01    16:dist 6:len : dist(5..4 + 1<<16), len(5..4 + 1<<6)
        //   esc 10    22:dist 8:len : dist(6..5 + 1<<22), len(6..5 + 1<<8)
        if (dist >= 4 && dist < 4 + (1 << 10) && len >= 4)
        {
            attempt.Clear();
            attempt.m_len = std::min(len, 3u + (1 << 4));
            attempt.m_bits.Push(escape, 8u);
            attempt.m_bits.Push(0, 2u);
            attempt.m_bits.Push(dist-4, 10u);
            attempt.m_bits.Push(attempt.m_len - 4, 4u);
            UpdateChoice();
        }
        if (dist >= 5 && dist < 5 + (1 << 16) && len >= 5)
        {
            attempt.Clear();
            attempt.m_len = std::min(len, 4u + (1 << 6));
            attempt.m_bits.Push(escape, 8u);
            attempt.m_bits.Push(1, 2u);
            attempt.m_bits.Push(dist - 5, 16u);
            attempt.m_bits.Push(attempt.m_len - 5, 6u);
            UpdateChoice();
        }
        if (dist >= 6 && dist < 6 + (1 << 22) && len >= 6)
        {
            attempt.Clear();
            attempt.m_len = std::min(len, 5u + (1 << 8));
            attempt.m_bits.Push(escape, 8u);
            attempt.m_bits.Push(2, 2u);
            attempt.m_bits.Push(dist - 6, 22u);
            attempt.m_bits.Push(attempt.m_len - 6, 8u);
            UpdateChoice();
        }
    };
    while(m_index < m_window.size())
    {
        // just write directly
        if (m_window[m_index] == escape)
        {
            choice.Set(escape, 255u);
        }
        else
        {
            choice.Set(m_window[m_index]);
        }
        // find sequence at some distance
        if (m_index + 4 < m_window.size())
        {
            Key key;
            key.Fill(m_window.begin()+m_index);
            // get the indices where the key appears
            auto indexIter = m_indices.find(key);
            if (indexIter == m_indices.end())
            {
                indexIter = m_indices.emplace(key, std::vector<uint64_t>()).first;
            }
            auto& indices = indexIter->second;
            // remove all indices which are out of range
            auto iter = std::partition(indices.begin(), indices.end(), [&](const auto& index) { return index + 5 + (1<<22) >= m_index; });
            indices.erase(iter, indices.end());
            // try the available indices.
            for (const auto index : indices)
            {
                unsigned int maxlen = 5 + (1 << 8);
                unsigned int len = 4;
                unsigned int currlen = 0;
                for (; currlen < maxlen && currlen + m_index < m_window.size(); ++currlen)
                {
                    if (m_window[m_index + currlen] != m_window[index + currlen])
                    {
                        break;
                    }
                }
                if (currlen > len)
                {
                    len = currlen;
                    SaveDistLen((unsigned int)(m_index-index), len);
                }
            }
            // add the new key
            indices.emplace_back(m_index);
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
    bits.Push(3u, 8u);
    bits.Pop(ioBuffer);
}

void WindowDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
    auto CopySequence = [&](unsigned int dist, unsigned int len)
    {
        auto index = m_window.size();
        for (unsigned int i = 0; i < len; ++i)
        {
            m_window.push_back(m_window[index-dist+i]);
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
                    m_window.emplace_back(c);
                }
                return true;
            }
            else
            {
                auto code = m_input.Peek(2u);
                switch (code)
                {
                case 0: //   esc 00    10:dist 4:len : dist(4..3 + 1<<10), len(4..3 + 1<<4)
                    if (m_input.Size() > 8 * 2)
                    {
                        m_input.Pop(2u);
                        auto dist = m_input.Pop(10u) + 4;
                        auto len = m_input.Pop(4u) + 4;
                        CopySequence(dist, len);
                        m_escaped = false;
                        return true;
                    }
                    break;
                case 1: //   esc 01    16:dist 6:len : dist(5..4 + 1<<16), len(5..4 + 1<<6)
                    if (m_input.Size() > 8 * 3)
                    {
                        m_input.Pop(2u);
                        auto dist = m_input.Pop(16u) + 5;
                        auto len = m_input.Pop(6u) + 5;
                        CopySequence(dist, len);
                        m_escaped = false;
                        return true;
                    }
                    break;
                case 2: //   esc 10    22:dist 8:len : dist(6..5 + 1<<22), len(6..5 + 1<<8)
                    if (m_input.Size() > 8 * 4)
                    {
                        m_input.Pop(2u);
                        auto dist = m_input.Pop(22u) + 6;
                        auto len = m_input.Pop(8u) + 6;
                        CopySequence(dist, len);
                        m_escaped = false;
                        return true;
                    }
                    break;
                case 3: //   esc 11 : esc/EOF
                    {
                        code = m_input.Pop(8u);
                        assert(code == 255 || code == 3);
                        m_eof = (code == 3);
                        m_escaped = false;
                        if (!m_eof)
                        {
                            m_window.push_back(escape);
                        }
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
    ioBuffer.insert(ioBuffer.end(), m_window.begin(), m_window.end());
    m_window.clear();
}
