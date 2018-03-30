#pragma once

#include <vector>
#include <memory>

class ICompressor
{
public:
    virtual void Compress(std::vector<unsigned char>& ioBuffer) = 0;
    virtual void Finish(std::vector<unsigned char>& ioBuffer) = 0;
};

class IDeCompressor
{
public:
    virtual void DeCompress(std::vector<unsigned char>& ioBuffer) = 0;
    virtual void Finish(std::vector<unsigned char>& ioBuffer) = 0;
};

enum class CompressorType
{
    PassThrough,
    RLE,
    Window,
    StaticHuffman,
    RLE_StaticHuffman,
};

class CompressorFactory
{
public:
    static std::shared_ptr<ICompressor> Create(const CompressorType compressorType);
};

class DeCompressorFactory
{
public:
    static std::shared_ptr<IDeCompressor> Create(const CompressorType compressorType);
};

