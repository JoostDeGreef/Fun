#pragma once

#include "ICompress.h"

class StaticHuffmanCompressor : public ICompressor
{
public:
    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;
private:
};

class StaticHuffmanDeCompressor : public IDeCompressor
{
public:
    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;
private:
};

