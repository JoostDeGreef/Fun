#include <assert.h>

#include "ICompress.h"

#include "PassThrough.h"
#include "StaticHuffman.h"

std::shared_ptr<ICompressor> CompressorFactory::Create(const CompressorType compressorType)
{
    switch (compressorType)
    {
    case CompressorType::PassThrough: return std::make_shared<PassThroughCompressor>();
    case CompressorType::StaticHuffman: return std::make_shared<StaticHuffmanCompressor>();
    }
    assert(false);
    return std::shared_ptr<ICompressor>();
}

std::shared_ptr<IDeCompressor> DeCompressorFactory::Create(const CompressorType compressorType)
{
    switch (compressorType)
    {
    case CompressorType::PassThrough: return std::make_shared<PassThroughDeCompressor>();
    case CompressorType::StaticHuffman: return std::make_shared<StaticHuffmanDeCompressor>();
    }
    assert(false);
    return std::shared_ptr<IDeCompressor>();
}
