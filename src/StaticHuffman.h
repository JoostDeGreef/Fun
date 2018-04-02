#pragma once

#include "BitBuffer.h"
#include "ICompress.h"

// bit stream format:
//   - write table
//   - write keys
//   - write end

class StaticHuffmanCommon
{
protected:
    static const unsigned int keyEnd = 256;

    StaticHuffmanCommon();

    void BuildTree();

    typedef std::array<size_t, 256> CountTable;
    CountTable m_counts;

    struct Key
    {
        unsigned int value;
        size_t length;
        std::vector<unsigned char> bits;
    };
    std::array<Key, 256 + 1> m_keys;

    enum class NodeType
    {
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
    std::array<Node, (256 + 1) * 2> m_treeCache;
    Node& m_tree;
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

