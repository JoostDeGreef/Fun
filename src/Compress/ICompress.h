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

class CompressorFactory
{
public:
    static std::shared_ptr<ICompressor> Create();
};

class DeCompressorFactory
{
public:
    static std::shared_ptr<IDeCompressor> Create();
};

