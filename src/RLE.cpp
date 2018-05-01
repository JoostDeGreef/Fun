#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "RLE.h"

const unsigned char RLECommon::m_escape = 255;
 
void RLECompressor::DumpCurrent(std::vector<unsigned char>& buffer)
{
    while (m_count > 0)
    {
        if (m_count == 1)
        {
            if (m_escape == m_current)
            {
                buffer.emplace_back(m_current);
            }
            buffer.emplace_back(m_current);
            m_count = 0;
            return;
        }
        else if (m_count == 2)
        {
            if (m_escape == m_current && m_escape != m_count)
            {
                for (decltype(m_count) i = 0; i < m_count; ++i)
                {
                    buffer.emplace_back(m_current);
                }
                m_count = 0;
                return;
            }
        }
        // todo: smarter split if m_count>255
        auto run = static_cast<unsigned char>(std::min(m_count,static_cast<decltype(m_count)>(255)));
        if (run == m_escape)
        {
            --run;
        }
        buffer.emplace_back(m_escape);
        buffer.emplace_back(run);
        buffer.emplace_back(m_current);
        m_count -= run;
    }
}

void RLECompressor::Compress(std::vector<unsigned char>& buffer)
{
    if (m_buffer.capacity()*4 < buffer.capacity()*3)
    {
        m_buffer.reserve(buffer.capacity());
    }
    for (const auto c : buffer)
    {
        if (m_count > 0)
        {
            if (m_current == c)
            {
                ++m_count;
            }
            else
            {
                DumpCurrent(m_buffer);
                m_count = 1;
                m_current = c;
            }
        }
        else
        {
            m_count = 1;
            m_current = c;
        }
    }
    m_buffer.swap(buffer);
    m_buffer.clear();
}

void RLECompressor::Finish(std::vector<unsigned char>& buffer)
{
    Compress(buffer);
    DumpCurrent(buffer);
    m_current = 0;
}

void RLEDeCompressor::DeCompress(std::vector<unsigned char>& buffer)
{
    if (m_buffer.capacity() * 4 < buffer.capacity() * 3)
    {
        m_buffer.reserve(buffer.capacity());
    }
    for (const auto c : buffer)
    {
        if (m_escaped)
        {
            if (m_escape == c && m_count==0)
            {
                m_buffer.emplace_back(m_escape);
                m_escaped = false;
            }
            else if(m_count==0)
            {
                m_count = c;
            }
            else
            {
                for (; m_count != 0; --m_count)
                {
                    m_buffer.emplace_back(c);
                }
                assert(0 == m_count);
                m_escaped = false;
            }
        }
        else
        {
            if (m_escape == c)
            {
                m_escaped = true;
            }
            else
            {
                m_buffer.emplace_back(c);
            }
        }
    }
    m_buffer.swap(buffer);
    m_buffer.clear();
}

void RLEDeCompressor::Finish(std::vector<unsigned char>& buffer)
{
    DeCompress(buffer);
    if (m_escaped)
    {
        throw std::runtime_error("Incomplete data");
    }
}


