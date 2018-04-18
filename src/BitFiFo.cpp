#include <algorithm>
#include <cassert>

#include "BitFiFo.h"

bool BitFiFo::TryPeek(unsigned int& data, size_t bits)
{
    static const unsigned int mask[] =
    {
        (1u <<  0) - 1,  (1u <<  1) - 1,  (1u <<  2) - 1,  (1u <<  3) - 1,  (1u <<  4) - 1,  (1u <<  5) - 1,  (1u <<  6) - 1,  (1u <<  7) - 1,
        (1u <<  8) - 1,  (1u <<  9) - 1,  (1u << 10) - 1,  (1u << 11) - 1,  (1u << 12) - 1,  (1u << 13) - 1,  (1u << 14) - 1,  (1u << 15) - 1,
        (1u << 16) - 1,  (1u << 17) - 1,  (1u << 18) - 1,  (1u << 19) - 1,  (1u << 20) - 1,  (1u << 21) - 1,  (1u << 22) - 1,  (1u << 23) - 1,
        (1u << 24) - 1,  (1u << 25) - 1,  (1u << 26) - 1,  (1u << 27) - 1,  (1u << 28) - 1,  (1u << 29) - 1,  (1u << 30) - 1,  (1u << 31) - 1
    };
    assert(bits <= sizeof(data) * 8);
    if (Size() >= bits)
    {
        const size_t index = m_begin / data_bits;
        const size_t firstOffset = m_begin % data_bits;
        size_t firstSize = firstOffset + bits;
        if (firstSize <= data_bits)
        {
            data = (m_data[index] >> firstOffset) & mask[bits];
        }
        else
        {
            firstSize = data_bits - firstOffset;
            data  = (m_data[index] >> firstOffset) & mask[firstSize];
            data |= m_data[index+1] & mask[bits-firstSize];
        }
        return true;
    }
    return false;
}

