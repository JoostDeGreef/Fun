#pragma once

#include "BitFiFo.h"
#include "ICompress.h"

// bit stream format:
//   - init with (new table:0),(end:1)
//   - repeat until done:
//       write table
//       write keys
//   - write end

template<typename Implements>
class DynamicHuffmanCommon : public Implements
{
protected:
    static const unsigned int keyNew = 256;
    static const unsigned int keyEnd = 257;

    static const unsigned int keyCount = 258;
    static const unsigned int startNodeBits = 5;

    // input/output buffer
    BitFiFo m_buffer;

    class KeyBits : public BitFiFo
    {
    public:
        size_t Length() const
        {
            return BitFiFo::Size();
        }
        operator BitFiFo() const
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
        KeyBits bits;
        void ClearBits()
        {
            bits.Clear();
        }
        void FillBits()
        {
            FillBits(node, bits);
        }
    private:
        inline static void FillBits(Node* n, KeyBits& bits)
        {
            if (n->parent)
            {
                FillBits(n->parent, bits);
                bits.PushBit(n->parent->node[0] != n);
            }
        }
    };
    struct Node
    {
        Node* parent;
        Node* before;
        unsigned int count;
        NodeType type;
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

    DynamicHuffmanCommon()
        : m_buffer()
        , m_nodeCache()
        , m_nodes()
        , m_keys()
        , m_tree(nullptr)
    {
        m_nodes.reserve(keyCount * 2);
        for (unsigned int key = 0; key<256; ++key)
        {
            m_keys[key].count = 0;
            m_keys[key].value = key;
        }
        for (auto key : { keyNew,keyEnd })
        {
            m_keys[key].count = 1;
            m_keys[key].value = key;
        }
        BuildTree();
    }

    // add a node to the end of m_nodes;
    inline Node* AddNode()
    {
        m_nodes.emplace_back(&m_nodeCache[m_nodes.size()]);
        return m_nodes.back();
    }

    bool UpdateTree(unsigned int key, const bool forceUpdate)
    {
        // TODO: something is wrong here...
        auto NodeOrderBroken = [](Node* node)
        {
            return (node->before != nullptr && node->count > node->before->count);
        };
        auto UpdateNode = [&NodeOrderBroken](Node* node)
        {
            bool res = false;
            while (node != nullptr)
            {
                node->count++;
                res = res || NodeOrderBroken(node);
                node = node->parent;
            }
            return !res;
        };
        // only update when there is a neighbor change detected
        m_keys[key].count++;
        if (forceUpdate)
        {
            BuildTree();
            return true;
        }
        else if( !UpdateNode(m_keys[key].node) )
        {
            BuildTree();
            return true;
        }
        return false;
    }

    void BuildTree()
    {
        m_nodes.clear();
        // add nodes for all used keys
        for (auto& key:m_keys)
        {
            if (key.count > 0)
            {
                key.ClearBits();
                Node* node = AddNode();
                node->SetKey(&key);
            }
        }
        // sort nodes, these are all 'key nodes'
        std::sort(m_nodes.begin(), m_nodes.end(), [](Node* a, Node* b)
        {
            assert(a->type == NodeType::leaf);
            assert(b->type == NodeType::leaf);
            return (a->count > b->count) || (a->count == b->count && a->key->value < b->key->value);
        });
        // repeatedly join the 2 least important nodes until there is one node left
        {
            Node* node = nullptr;
            for (size_t nodeCount = m_nodes.size(); nodeCount >= 2; --nodeCount)
            {
                assert(m_nodes.size() < m_nodeCache.size());
                node = &m_nodeCache[m_nodes.size()];
                node->SetNodes(m_nodes[nodeCount - 2], m_nodes[nodeCount - 1]);
                auto iter = std::lower_bound(m_nodes.begin(), m_nodes.begin() + nodeCount - 2, node, [](Node* a, Node* b)
                {
                    return (a->count > b->count);
                });
                m_nodes.emplace(iter, node);
            }
            m_tree = node;
        }
        // fill before
        {
            Node* prev = nullptr;
            for (auto& node : m_nodes)
            {
                node->before = prev;
                prev = node;
            }
        }
    }
};

class DynamicHuffmanCompressor : public DynamicHuffmanCommon<ICompressor>
{
public:
    DynamicHuffmanCompressor();

    void Compress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    void WriteKeyUsingTree(unsigned int key);
};

class DynamicHuffmanDeCompressor : public DynamicHuffmanCommon<IDeCompressor>
{
public:
    DynamicHuffmanDeCompressor();

    void DeCompress(std::vector<unsigned char>& ioBuffer) override;
    void Finish(std::vector<unsigned char>& ioBuffer) override;

private:
    Node * m_currentNode;
};

