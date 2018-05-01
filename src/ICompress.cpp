#include <cassert>
#include <array>
#include <deque>
#include <algorithm>

#include "ICompress.h"

#include "PassThrough.h"
#include "RLE.h"
#include "Window.h"
#include "StaticHuffman.h"
#include "StaticBlockHuffman.h"
#include "DynamicHuffman.h"
#include "PipeLine.h"

std::shared_ptr<ICompressor> CompressorFactory::Create(const CompressorType compressorType)
{
    switch (compressorType)
    {
    case CompressorType::PassThrough:            return std::make_shared<PassThroughCompressor>();
    case CompressorType::RLE:                    return std::make_shared<RLECompressor>();
    case CompressorType::Window:                 return std::make_shared<WindowCompressor>();
    case CompressorType::StaticHuffman:          return std::make_shared<StaticHuffmanCompressor>();
    case CompressorType::StaticBlockHuffman:     return std::make_shared<StaticBlockHuffmanCompressor>();
    case CompressorType::DynamicHuffman:         return std::make_shared<DynamicHuffmanCompressor>();
    case CompressorType::RLE_StaticHuffman:      return std::make_shared<PipeLineCompressor<RLECompressor, StaticHuffmanCompressor>>();
    case CompressorType::RLE_StaticBlockHuffman: return std::make_shared<PipeLineCompressor<RLECompressor, StaticBlockHuffmanCompressor>>();
    case CompressorType::RLE_DynamicHuffman:     return std::make_shared<PipeLineCompressor<RLECompressor, DynamicHuffmanCompressor>>();
    }
    assert(false);
    return std::shared_ptr<ICompressor>();
}

std::shared_ptr<IDeCompressor> DeCompressorFactory::Create(const CompressorType compressorType)
{
    switch (compressorType)
    {
    case CompressorType::PassThrough:            return std::make_shared<PassThroughDeCompressor>();
    case CompressorType::RLE:                    return std::make_shared<RLEDeCompressor>();
    case CompressorType::Window:                 return std::make_shared<WindowDeCompressor>();
    case CompressorType::StaticHuffman:          return std::make_shared<StaticHuffmanDeCompressor>();
    case CompressorType::StaticBlockHuffman:     return std::make_shared<StaticBlockHuffmanDeCompressor>();
    case CompressorType::DynamicHuffman:         return std::make_shared<DynamicHuffmanDeCompressor>();
    case CompressorType::RLE_StaticHuffman:      return std::make_shared<PipeLineDeCompressor<RLEDeCompressor, StaticHuffmanDeCompressor>>();
    case CompressorType::RLE_StaticBlockHuffman: return std::make_shared<PipeLineDeCompressor<RLEDeCompressor, StaticBlockHuffmanDeCompressor>>();
    case CompressorType::RLE_DynamicHuffman:     return std::make_shared<PipeLineDeCompressor<RLEDeCompressor, DynamicHuffmanDeCompressor>>();
    }
    assert(false);
    return std::shared_ptr<IDeCompressor>();
}
