#pragma once

#include "ICompress.h"

// format (lsb->msb)
//   esc 11 11 11 11         : esc
//   esc 11 00 00 00         : EOF
//   esc 00    10:dist 4:len : dist(4..3 + 1<<10), len(4..3 + 1<<4)
//   esc 01    16:dist 6:len : dist(5..4 + 1<<16), len(5..4 + 1<<6)
//   esc 10    22:dist 8:len : dist(6..5 + 1<<22), len(6..5 + 1<<8)

template<typename IMPLEMENTS>
class WindowCommon : public IMPLEMENTS
{
protected:
    static const unsigned char escape = 255;

    WindowCommon()
        : m_window()
    {}

    std::vector<unsigned char> m_window;
};

class WindowCompressor : public WindowCommon<ICompressor>
{
public:
    WindowCompressor()
        : WindowCommon()
        , m_indices()
    {}

    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;
private:
    class Key
    {
    public:
        unsigned char operator[] (unsigned int index) { return m_data[index]; }

        template<typename ITERATOR>
        void Fill(ITERATOR iter)
        {
            for (auto& element : m_data)
            {
                element = *iter++;
            }
        }

        bool operator < (const Key& other) const
        {
            return m_data < other.m_data;
        }
    private:
        std::array<unsigned char, 4> m_data;
    };

    std::map<Key, std::vector<uint64_t>> m_indices;

    uint64_t m_index = 0;
    uint64_t m_offset = 0;
};

class WindowDeCompressor : public WindowCommon<IDeCompressor>
{
public:
    WindowDeCompressor()
        : WindowCommon()
        , m_input()
        , m_eof(false)
        , m_escaped(false)
    {}

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;
private:
    BitFiFo m_input;
    bool m_eof;
    bool m_escaped;
};

