#include <cassert>
#include <array>
#include <deque>
#include <algorithm>

#include "ICompress.h"
#include "BitFiFo.h"

#include "RLE.h"
#include "DynamicHuffman.h"
#include "PipeLine.h"

std::shared_ptr<ICompressor> CompressorFactory::Create()
{
//  return std::make_shared<PipeLineCompressor<RLECompressor, DynamicHuffmanCompressor>>();
//	return std::make_shared<RLECompressor>();
	return std::make_shared<DynamicHuffmanCompressor>();
}

std::shared_ptr<IDeCompressor> DeCompressorFactory::Create()
{
//  return std::make_shared<PipeLineDeCompressor<RLEDeCompressor, DynamicHuffmanDeCompressor>>();
//	return std::make_shared<RLEDeCompressor>();
	return std::make_shared<DynamicHuffmanDeCompressor>();
}
