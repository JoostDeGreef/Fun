#pragma once

#include <memory>
#include <vector>
#include <deque>

class BitStream
{
private:

public:
    BitStream();

    void Push(unsigned int data, unsigned int bits);

    unsigned int Pop(const unsigned int bits);
    bool TryPop(unsigned int& data, unsigned int bits);
    
    void FlushFront(); // make sure the buffer starts at a whole byte
    void FlushBack(); // fill the buffer with 0 so it ends on a whole byte

    size_t BitsAvailable() const;

private:
    std::deque<unsigned char> m_bigBuffer;
    unsigned int m_frontBuffer;
    unsigned int m_frontBits;
    unsigned int m_backBuffer;
    unsigned int m_backBits;
};
