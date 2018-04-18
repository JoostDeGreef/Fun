#pragma once

#include "BitFiFo.h"
#include "ICompress.h"
#include "HuffmanCommon.h"

// bit stream format:
//   - init with (new table:0),(end:1)
//   - repeat until done:
//       write table
//       write keys
//   - write end

class StaticBlockHuffmanCommon : public HuffmanCommon<258>
{
protected:
    static const unsigned int keyTable = 256;
    static const unsigned int keyEnd = 257;

    static const size_t blockSize = 128;
    static const size_t initialBlocks = 8;
    static const size_t maxBlocks = 512;   // limits the memory usage while encoding
    static const unsigned int startNodeBits = 4;
    static const double diffTrigger;

    StaticBlockHuffmanCommon();

    BitFiFo m_buffer;
};

class StaticBlockHuffmanCompressor : public ICompressor, StaticBlockHuffmanCommon
{
public:
    StaticBlockHuffmanCompressor();

    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    void WriteTree();
    void WriteKeyUsingTree(unsigned int key);

    Counts m_newCounts;
    std::deque<unsigned char> m_inBuffer;
};

class StaticBlockHuffmanDeCompressor : public IDeCompressor, StaticBlockHuffmanCommon
{
public:
    StaticBlockHuffmanDeCompressor();

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    bool ReadTree();
    void FillStartNodes();

    Nodes m_startNodes;
    Node* m_currentNode;
};

