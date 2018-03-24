#include <array>
#include <deque>

#include "StaticHuffman.h"

const double StaticHuffmanCommon::diffTrigger = 0.1;

StaticHuffmanCommon::StaticHuffmanCommon()
    : m_counts()
    , m_buffer()
{
    m_counts.fill(0);
    BuildTree();
}

void StaticHuffmanCommon::BuildTree()
{
    for (auto& key : m_keys)
    {
        key.length = 0;
        key.bits.clear();
    }
    // todo
}


StaticHuffmanCompressor::StaticHuffmanCompressor()
    : StaticHuffmanCommon()
{
    m_newCounts.fill(0);
}

void StaticHuffmanCompressor::WriteKeyUsingTree(unsigned int key)
{
    // todo
}

void StaticHuffmanCompressor::WriteTree()
{
    // todo
}

void StaticHuffmanCompressor::Compress(std::vector<unsigned char>& ioBuffer)
{
    for (size_t i = 0; i < ioBuffer.size(); ++i)
    {
        const auto c = ioBuffer[i];
        ++m_newCounts[c];
        m_inBuffer.emplace_back(c);
        if (m_inBuffer.size() % blockSize == 0)
        {
            if (m_inBuffer.size() < initialBlocks * blockSize)
            {
                // do nothing
            }
            else if (m_inBuffer.size() == initialBlocks * blockSize)
            {
                m_newCounts.swap(m_counts);
            }
            else
            {
                // if the distribution in newcounts differs too much from counts, store counts and process the bytes read for that
                const double ratioCount = 1.0/(m_inBuffer.size() - blockSize);
                const double ratioNewCount = 1.0/blockSize;
                double diff = 0;
                for (size_t i = 0; i < 256; ++i)
                {
                    diff += abs(m_counts[i] * ratioCount - m_newCounts[i] * ratioNewCount);
                }
                if (diff < diffTrigger)
                {
                    for (size_t i = 0; i < 256; ++i)
                    {
                        m_counts[i] += m_newCounts[i];
                    }
                    m_newCounts.fill(0);
                }
                else
                {
                    WriteKeyUsingTree(keyTable);
                    BuildTree();
                    WriteTree();
                    for (size_t i = 0; i < 256; ++i)
                    {
                        m_counts[i] = m_newCounts[i];
                    }
                    m_newCounts.fill(0);
                    while (m_inBuffer.size() > blockSize)
                    {
                        WriteKeyUsingTree(m_inBuffer.front());
                        m_inBuffer.pop_front();
                    }
                }
            }
        }
    }
    ioBuffer.clear();
    m_buffer.RetrieveFrontBytes(ioBuffer);        
}

void StaticHuffmanCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    Compress(ioBuffer);
}


void StaticHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
}

void StaticHuffmanDeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
}
