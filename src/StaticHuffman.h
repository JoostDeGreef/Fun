#pragma once

#include "BitBuffer.h"
#include "ICompress.h"
#include "HuffmanCommon.h"

// bit stream format:
//   - write table
//   - write keys
//   - write end

class StaticHuffmanCommon : public HuffmanCommon<257>
{
protected:
    static const unsigned int keyEnd = 256;

    StaticHuffmanCommon();
};

class StaticHuffmanCompressor : public ICompressor, StaticHuffmanCommon
{
public:
    StaticHuffmanCompressor();

    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    void WriteTree(BitBuffer& buffer) const;
    void WriteKeyUsingTree(BitBuffer& buffer, unsigned int key) const;

    std::deque<unsigned char> m_inBuffer;
};

class StaticHuffmanDeCompressor : public IDeCompressor, StaticHuffmanCommon
{
public:
    StaticHuffmanDeCompressor();

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    bool ReadTree();
    Node* m_currentNode;
    BitBuffer m_inBuffer;
};

