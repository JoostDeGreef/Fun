#pragma once

#include "ICompress.h"

// format (lsb->msb)
//   esc 00 00  4:len        : (len+1)*esc
//   esc 00 01  4:len 8:val  : (len+1)*val
//   esc 00 10 00 00         : EOF
//   esc 00 11 20:dist 8:len : len(6-261)
//   esc 01     4:dist 2:len : len(3-6)
//   esc 10    10:dist 4:len : len(4-19)
//   esc 11    16:dist 6:len : len(5-36)

template<typename IMPLEMENTS>
class WindowCommon : public IMPLEMENTS
{
protected:
    static const unsigned char escape = 255;

    WindowCommon()
        : m_buffer()
    {}

    std::vector<unsigned char> m_buffer;
};

class WindowCompressor : public WindowCommon<ICompressor>
{
public:
    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;
private:
    unsigned int m_index = 0;
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

