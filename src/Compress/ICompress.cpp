#include <cassert>
#include <array>
#include <deque>
#include <algorithm>

#include "ICompress.h"
#include "BitFiFo.h"

#include "RLE.h"
#include "Window.h"
#include "DynamicHuffman.h"
#include "StaticHuffman.h"
#include "PipeLine.h"

std::shared_ptr<ICompressor> CompressorFactory::Create(CompressionAlgo ca)
{
    switch (ca)
    {
    default:
    case CompressionAlgo::DynamicHuffman:
        return std::make_shared<DynamicHuffmanCompressor>();
    case CompressionAlgo::StaticHuffman:
        return std::make_shared<StaticHuffmanCompressor>();
    case CompressionAlgo::Window:
        return std::make_shared<WindowCompressor>();
    case CompressionAlgo::RLE:
        return std::make_shared<RLECompressor>();
    case CompressionAlgo::RLE_DynamicHuffman:
        return std::make_shared<PipeLineCompressor<RLECompressor, DynamicHuffmanCompressor>>();
    case CompressionAlgo::RLE_StaticHuffman:
        return std::make_shared<PipeLineCompressor<RLECompressor, StaticHuffmanCompressor>>();
    case CompressionAlgo::Window_DynamicHuffman:
        return std::make_shared<PipeLineCompressor<WindowCompressor, DynamicHuffmanCompressor>>();
    case CompressionAlgo::Window_RLE_DynamicHuffman:
        return std::make_shared<PipeLineCompressor<WindowCompressor, RLECompressor, DynamicHuffmanCompressor>>();
    }
}

std::shared_ptr<IDeCompressor> DeCompressorFactory::Create(CompressionAlgo ca)
{
    switch (ca)
    {
    default:
    case CompressionAlgo::DynamicHuffman:
        return std::make_shared<DynamicHuffmanDeCompressor>();
    case CompressionAlgo::StaticHuffman:
        return std::make_shared<StaticHuffmanDeCompressor>();
    case CompressionAlgo::Window:
        return std::make_shared<WindowDeCompressor>();
    case CompressionAlgo::RLE:
        return std::make_shared<RLEDeCompressor>();
    case CompressionAlgo::RLE_DynamicHuffman:
        return std::make_shared<PipeLineDeCompressor<RLEDeCompressor, DynamicHuffmanDeCompressor>>();
    case CompressionAlgo::RLE_StaticHuffman:
        return std::make_shared<PipeLineDeCompressor<RLEDeCompressor, StaticHuffmanDeCompressor>>();
    case CompressionAlgo::Window_DynamicHuffman:
        return std::make_shared<PipeLineDeCompressor<WindowDeCompressor, DynamicHuffmanDeCompressor>>();
    case CompressionAlgo::Window_RLE_DynamicHuffman:
        return std::make_shared<PipeLineDeCompressor<WindowDeCompressor, RLEDeCompressor, DynamicHuffmanDeCompressor>>();
    }
}
