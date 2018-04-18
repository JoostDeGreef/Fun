#pragma once

#include "BitFiFo.h"
#include "ICompress.h"

// bit stream format:
//   - repeat for each 'blocksize'
//     - write table
//     - write keys
//   - write end

class StaticHuffmanCommon 
{
protected:
    static const unsigned int keyEnd = 256;
    static const unsigned int keyCount = 257;
    static const unsigned int blockSize = 16384;
    static const unsigned int startNodeBits = 4;

    enum class NodeType
    {
        branch,
        leaf
    };
    struct Node
    {
        NodeType type;
        unsigned int count;
        unsigned int depth;
        union
        {
            Node* node[2];
            unsigned int key;
        };
    };
    typedef std::array<Node, keyCount * 2> NodeCache;
    NodeCache m_nodeCache;
    typedef std::vector<Node*> Nodes;
    Node* m_tree;
};

class StaticHuffmanCompressor : public ICompressor, StaticHuffmanCommon
{
public:
    StaticHuffmanCompressor();

    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    struct Key
    {
        uint64_t count;
        BitFiFo bits;
    };
    typedef std::array<Key, keyCount> Keys;
    Keys m_keys;

    void ClearKeys();
    void BuildTree();
    void CompressBuffer(std::vector<unsigned char>::const_iterator begin, std::vector<unsigned char>::const_iterator end);

    void WriteTree(BitFiFo& buffer) const;
    void WriteKeyUsingTree(BitFiFo& buffer, unsigned int key) const;

    Nodes m_nodes;
    std::vector<unsigned char> m_inBuffer;
    BitFiFo m_outBuffer;
};

class StaticHuffmanDeCompressor : public IDeCompressor, StaticHuffmanCommon
{
public:
    StaticHuffmanDeCompressor();

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    bool ReadTree();
    void FillStartNodes();

    Nodes m_startNodes;
    Node* m_currentNode;
    BitFiFo m_inBuffer;
    unsigned int m_blockCount;
};

