#pragma once

#include "BitBuffer.h"
#include "ICompress.h"

// bit stream format:
//   - init with (new table:0),(end:1)
//   - repeat until done:
//       write table
//       write keys
//   - write end

class DynamicHuffman2Common
{
protected:
    static const unsigned int keyNew = 256;
    static const unsigned int keyEnd = 257;

    static const unsigned int keyCount = 258;

    // bytes in history
    static const unsigned int lifetime = 16384;

    // input/output buffer
    BitBuffer m_buffer;

    // byte history
    std::array<unsigned int, lifetime> m_history;
    unsigned int m_historyIndex;

    class KeyBits : public BitBuffer
    {
    public:
        size_t Length() const
        {
            return BitBuffer::BitsAvailable();
        }
        operator BitBuffer() const
        {
            return *this;
        }
    };
    enum class NodeType
    {
        branch,
        leaf
    };
    struct Node;
    struct Key
    {
        Node* node = nullptr;
        unsigned int value = 0;
        unsigned int count = 0;
        // todo: optimize bits?
        KeyBits bits;
        void ClearBits()
        {
            bits.Clear();
        }
        void FillBits()
        {
            std::vector<unsigned char> nodeBits;
            Node* n = node;
            while (n->parent != nullptr)
            {
                nodeBits.push_back(n->parent->node[0] == n ? 0 : 1);
                n = n->parent;
            } 
            for(auto iter=nodeBits.crbegin();iter!=nodeBits.crend();++iter)
            {
                bits.Push(*iter, 1u);
            }
        }
    };
    struct Node
    {
        NodeType type;
        unsigned int count;
        Node* parent;
        Node* before;
        Node* after;
        union
        {
            Node* node[2];
            Key* key;
        };

        void SetKey(Key* k)
        {
            type = NodeType::leaf;
            key = k;
            count = k->count;
            k->node = this;
        }
        void SetNodes(Node* n0, Node* n1)
        {
            type = NodeType::branch;
            node[0] = n0;
            node[1] = n1;
            n0->parent = this;
            n1->parent = this;
            count = n0->count + n1->count;
        }
    };

    typedef std::array<Node, keyCount * 2> NodeCache;
    NodeCache m_nodeCache;

    typedef std::vector<Node*> Nodes;
    Nodes m_nodes;

    typedef std::array<Key, keyCount> Keys;
    Keys m_keys;

    Node* m_tree;

    DynamicHuffman2Common()
        : m_buffer()
        , m_history()
        , m_historyIndex(0)
        , m_nodeCache()
        , m_nodes()
        , m_keys()
        , m_tree(nullptr)
    {
        m_nodes.reserve(keyCount * 2);
        for (unsigned int key=0;key<256;++key)
        {
            m_keys[key].count = 0;
            m_keys[key].value = key;
        }
        for (auto key : { keyNew,keyEnd })
        {
            m_keys[key].count = 1;
            m_keys[key].value = key;
        }
        m_history.fill(-1);
        BuildTree();
    }

    // add a node to the end of m_nodes;
    Node* AddNode()
    {
        m_nodes.emplace_back(&m_nodeCache[m_nodes.size()]);
        return m_nodes.back();
    }

    void UpdateHistory(unsigned int key);

    void BuildTree();
};

class DynamicHuffman2Compressor : public ICompressor, DynamicHuffman2Common
{
public:
    DynamicHuffman2Compressor();

    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    void WriteKeyUsingTree(unsigned int key);
};

class DynamicHuffman2DeCompressor : public IDeCompressor, DynamicHuffman2Common
{
public:
    DynamicHuffman2DeCompressor();

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    Node* m_currentNode;
};

