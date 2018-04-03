#pragma once

#include "BitBuffer.h"
#include "ICompress.h"
#include "HuffmanCommon.h"

// bit stream format:
//   - init with (new table:0),(end:1)
//   - repeat until done:
//       write table
//       write keys
//   - write end

class DynamicHuffmanCommon : public HuffmanCommon<258>
{
protected:
    static const unsigned int keyNew = 256;
    static const unsigned int keyEnd = 257;

    static const unsigned int lifetime = 1000;

    DynamicHuffmanCommon();

    void UpdateHistory(unsigned int key);

    BitBuffer m_buffer;
    std::array<unsigned int, lifetime> m_history;
    unsigned int m_historyIndex;
};

class DynamicHuffmanCompressor : public ICompressor, DynamicHuffmanCommon
{
public:
    DynamicHuffmanCompressor();

    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    void WriteKeyUsingTree(unsigned int key);
};

class DynamicHuffmanDeCompressor : public IDeCompressor, DynamicHuffmanCommon
{
public:
    DynamicHuffmanDeCompressor();

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    Node* m_currentNode;
};

