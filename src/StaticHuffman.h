#pragma once

#include "BitBuffer.h"
#include "ICompress.h"

// bit stream format:
//   - init with (new table:0),(end:1)
//   - repeat untill done:
//       write table
//       write keys
//   - write end

class StaticHuffmanCommon
{
protected:
    static const unsigned int keyTable = 256;
    static const unsigned int keyEnd = 257;

    static const size_t blockSize = 128;
    static const size_t initialBlocks = 8;
    static const double diffTrigger;

    StaticHuffmanCommon();

    void BuildTree();

    typedef std::array<size_t, 256> CountTable;
    CountTable m_counts;

    BitBuffer m_buffer;

    struct Key
    {
        size_t length;
        std::vector<unsigned char> bits;
    };
    std::array<Key, 256 + 2> m_keys;

    enum class NodeType
    {
        root,
        branch,
        leaf
    };
    struct Node
    {
        NodeType type;
        union
        {
            Node* node[2];
            unsigned int leaf;
        };
    };
    std::array<Node, (256 + 2) * 2> m_treeCache;
    Node m_tree;
};

class StaticHuffmanCompressor : public ICompressor, StaticHuffmanCommon
{
public:
    StaticHuffmanCompressor();

    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    void WriteTree();
    void WriteKeyUsingTree(unsigned int key);

    CountTable m_newCounts;
    std::deque<unsigned char> m_inBuffer;
};

class StaticHuffmanDeCompressor : public IDeCompressor, StaticHuffmanCommon
{
public:

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
};

