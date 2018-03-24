#include <cassert>
#include <array>
#include <deque>

#include "ICompress.h"

#include "PassThrough.h"
#include "RLE.h"
#include "Window.h"
#include "StaticHuffman.h"

std::shared_ptr<ICompressor> CompressorFactory::Create(const CompressorType compressorType)
{
    switch (compressorType)
    {
    case CompressorType::PassThrough:   return std::make_shared<PassThroughCompressor>();
    case CompressorType::RLE:           return std::make_shared<RLECompressor>();
    case CompressorType::Window:        return std::make_shared<WindowCompressor>();
    case CompressorType::StaticHuffman: return std::make_shared<StaticHuffmanCompressor>();
    }
    assert(false);
    return std::shared_ptr<ICompressor>();
}

std::shared_ptr<IDeCompressor> DeCompressorFactory::Create(const CompressorType compressorType)
{
    switch (compressorType)
    {
    case CompressorType::PassThrough:   return std::make_shared<PassThroughDeCompressor>();
    case CompressorType::RLE:           return std::make_shared<RLEDeCompressor>();
    case CompressorType::Window:        return std::make_shared<WindowDeCompressor>();
    case CompressorType::StaticHuffman: return std::make_shared<StaticHuffmanDeCompressor>();
    }
    assert(false);
    return std::shared_ptr<IDeCompressor>();
}
