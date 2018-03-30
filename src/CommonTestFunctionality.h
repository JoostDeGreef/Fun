#pragma once

#include "gtest/gtest.h"
using namespace testing;

#include "ICompress.h"

inline std::ostream& operator<<(std::ostream& stream, CompressorType const& ct)
{
    switch (ct)
    {
    case CompressorType::PassThrough:       return stream << "PassThrough";
    case CompressorType::RLE:               return stream << "RLE";
    case CompressorType::Window:            return stream << "Window";
    case CompressorType::StaticHuffman:     return stream << "StaticHuffman";
    case CompressorType::RLE_StaticHuffman: return stream << "RLE_StaticHuffman";
    default: 
        assert(false);
        return stream << "Unknown(" << static_cast<int>(ct) << ")";
    }
}

