#pragma once

#include <vector>
#include <memory>
#include <map>
#include <array>
#include <iterator>

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

enum class CompressionAlgo
{
    DynamicHuffman,
    StaticHuffman,
    Window,
    RLE,
    RLE_DynamicHuffman,
    RLE_StaticHuffman,
    Window_DynamicHuffman,
    Window_RLE_DynamicHuffman
};

class CompressorFactory
{
public:
    static std::shared_ptr<ICompressor> Create(CompressionAlgo ca);
};

class DeCompressorFactory
{
public:
    static std::shared_ptr<IDeCompressor> Create(CompressionAlgo ca);
};

