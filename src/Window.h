#pragma once

#include "ICompress.h"

class WindowCompressor : public ICompressor
{
public:
    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;
private:
};

class WindowDeCompressor : public IDeCompressor
{
public:
    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;
private:
};

