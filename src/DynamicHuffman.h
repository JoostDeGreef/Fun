#pragma once

#include "BitBuffer.h"
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
    static const unsigned int startNodeBits = 4;

    // input/output buffer
    BitBuffer m_buffer;

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
            for (auto iter = nodeBits.crbegin(); iter != nodeBits.crend(); ++iter)
            {
                bits.Push(*iter, 1u);
            }
        }
    };
    struct Node
    {
        NodeType type;
        unsigned int count;
        unsigned int depth;
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
    Node* AddNode()
    {
        m_nodes.emplace_back(&m_nodeCache[m_nodes.size()]);
        return m_nodes.back();
    }

    bool UpdateTree(unsigned int key, const bool forceUpdate)
    {
        auto NodeOrderBroken = [](Node* node)
        {
            bool res = false;
            res = res || (node->before != nullptr && node->count > node->before->count);
            res = res || (node->after != nullptr && node->after->count > node->count);
            return res;
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
        bool rebuildTree = !UpdateNode(m_keys[key].node);
        if (rebuildTree || forceUpdate)
        {
            // todo:
            //   - see if the rebuild can be avoided completely?
            //   - perform a partial tree rebuild only?
            BuildTree();
            return true;
        }
        return false;
    }

    void BuildTree()
    {
        m_nodes.clear();
        // add nodes for all used keys
        for (size_t i = 0; i < keyCount; ++i)
        {
            if (m_keys[i].count > 0)
            {
                m_keys[i].ClearBits();
                Node* node = AddNode();
                node->SetKey(&m_keys[i]);
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
        // fill before/after
        {
            Node* prev = nullptr;
            for (auto& node : m_nodes)
            {
                node->before = prev;
                if (nullptr != prev)
                {
                    prev->after = node;
                }
                prev = node;
            }
            prev->after = nullptr;
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
    void FillStartNodes();

    Nodes m_startNodes;
    Node * m_currentNode;
};

