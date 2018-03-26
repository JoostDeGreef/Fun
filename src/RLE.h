#pragma once

#include "ICompress.h"

// byte stream format:
//   value => value
//   escape + escape => escape
//   escape + nr + value => nr * value

class RLECommon
{
protected:
    static const unsigned char m_escape;
};


class RLECompressor : public ICompressor, RLECommon
{
public:
    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    void DumpCurrent(std::vector<unsigned char>& buffer);

    unsigned char m_current = 0;
    size_t m_count = 0;
    std::vector<unsigned char> m_buffer;
};


class RLEDeCompressor : public IDeCompressor, RLECommon
{
public:

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    bool m_escaped = false;
    size_t m_count = 0;
    std::vector<unsigned char> m_buffer;
};

