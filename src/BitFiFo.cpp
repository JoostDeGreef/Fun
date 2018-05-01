#include <algorithm>
#include <cassert>

#include "BitFiFo.h"

const unsigned int BitFiFo::mask[] =
{
    (1u <<  0) - 1,  (1u <<  1) - 1,  (1u <<  2) - 1,  (1u <<  3) - 1,  (1u <<  4) - 1,  (1u <<  5) - 1,  (1u <<  6) - 1,  (1u <<  7) - 1,
    (1u <<  8) - 1,  (1u <<  9) - 1,  (1u << 10) - 1,  (1u << 11) - 1,  (1u << 12) - 1,  (1u << 13) - 1,  (1u << 14) - 1,  (1u << 15) - 1,
    (1u << 16) - 1,  (1u << 17) - 1,  (1u << 18) - 1,  (1u << 19) - 1,  (1u << 20) - 1,  (1u << 21) - 1,  (1u << 22) - 1,  (1u << 23) - 1,
    (1u << 24) - 1,  (1u << 25) - 1,  (1u << 26) - 1,  (1u << 27) - 1,  (1u << 28) - 1,  (1u << 29) - 1,  (1u << 30) - 1,  (1u << 31) - 1
};
const uint64_t BitFiFo::bit_mask[] =
{
    (1ull <<  0ull),  (1ull <<  1ull),  (1ull <<  2ull),  (1ull <<  3ull),  (1ull <<  4ull),  (1ull <<  5ull),  (1ull <<  6ull),  (1ull <<  7ull),
    (1ull <<  8ull),  (1ull <<  9ull),  (1ull << 10ull),  (1ull << 11ull),  (1ull << 12ull),  (1ull << 13ull),  (1ull << 14ull),  (1ull << 15ull),
    (1ull << 16ull),  (1ull << 17ull),  (1ull << 18ull),  (1ull << 19ull),  (1ull << 20ull),  (1ull << 21ull),  (1ull << 22ull),  (1ull << 23ull),
    (1ull << 24ull),  (1ull << 25ull),  (1ull << 26ull),  (1ull << 27ull),  (1ull << 28ull),  (1ull << 29ull),  (1ull << 30ull),  (1ull << 31ull),
    (1ull << 32ull),  (1ull << 33ull),  (1ull << 34ull),  (1ull << 35ull),  (1ull << 36ull),  (1ull << 37ull),  (1ull << 38ull),  (1ull << 39ull),
    (1ull << 40ull),  (1ull << 41ull),  (1ull << 42ull),  (1ull << 43ull),  (1ull << 44ull),  (1ull << 45ull),  (1ull << 46ull),  (1ull << 47ull),
    (1ull << 48ull),  (1ull << 49ull),  (1ull << 50ull),  (1ull << 51ull),  (1ull << 52ull),  (1ull << 53ull),  (1ull << 54ull),  (1ull << 55ull),
    (1ull << 56ull),  (1ull << 57ull),  (1ull << 58ull),  (1ull << 59ull),  (1ull << 60ull),  (1ull << 61ull),  (1ull << 62ull),  (1ull << 63ull)
};

