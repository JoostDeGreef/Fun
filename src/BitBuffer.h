#pragma once

#include <memory>
#include <vector>
#include <deque>

class BitBuffer
{
private:

public:
    BitBuffer();

    void Swap(BitBuffer& other);

    void Push(unsigned int data, unsigned int bits);
    void Push(const std::vector<unsigned char>& data, unsigned int bits);
    void Push(const std::vector<unsigned int>& data, unsigned int bits);
    void Push(const BitBuffer& data);

    unsigned int Pop(const unsigned int bits);
    bool TryPop(unsigned int& data, unsigned int bits);

    unsigned int Peek(const unsigned int bits);
    bool TryPeek(unsigned int& data, unsigned int bits);

    void FlushFront(); // make sure the buffer starts at a whole byte
    void FlushBack(); // fill the buffer with 0 so it ends on a whole byte

    size_t BitsAvailable() const;

    bool Empty() const;
    bool HasData() const;
    
    void Clear();

    void RetrieveFrontInts(std::vector<unsigned int>& outBuffer);
    void RetrieveFrontBytes(std::vector<unsigned char>& outBuffer);
private:
    std::deque<unsigned int> m_bigBuffer;
    unsigned int m_frontBuffer;
    unsigned int m_frontBits;
    unsigned int m_backBuffer;
    unsigned int m_backBits;
};
