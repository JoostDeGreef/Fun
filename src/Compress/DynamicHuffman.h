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
        uint64_t tick;
        unsigned int index;
        unsigned int count;
        NodeType type;
        union
        {
            Node* node[2];
            Key* key;
        };
        void SetKey(Key* k)
        {
            tick = (uint64_t)-1;
            key = k;
            count = k->count;
            type = NodeType::leaf;
            k->node = this;
        }
        void SetNodes(Node* n0, Node* n1)
        {
            tick = (uint64_t)-1;
            node[0] = n0;
            node[1] = n1;
            count = n0->count + n1->count;
            type = NodeType::branch;
            n0->parent = this;
            n1->parent = this;
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

    void UpdateTree(unsigned int key, const bool forceUpdate)
    {
        struct LocalFunctions
        {
            // recount immediate children
            static inline void UpdateCount(Node* node)
            {
                do
                {
                    node->count = node->node[0]->count + node->node[1]->count;
                    node = node->parent;
                } 
                while (nullptr != node);
            }
            // clear the Key bits in this subtree, adjust node order for nodes which share a parent.
            static inline void UpdateSubtree(Node* node, const uint64_t tick)
            {
                assert(node->parent != node);
                if (node->tick == tick)
                {
                    return;
                }
                if (node->type == NodeType::branch)
                {
                    UpdateSubtree(node->node[0], tick);
                    UpdateSubtree(node->node[1], tick);
                    assert(node->count == node->node[0]->count + node->node[1]->count);
                }
                else
                {
                    node->key->ClearBits();
                    assert(node->count == node->key->count);
                }
            };
            // swap 2 nodes
            static inline void SwapNodes(const unsigned int ai, const unsigned int bi, Nodes& nodes, const uint64_t tick)
            {
                Node* a = nodes[ai];
                Node* b = nodes[bi];

                assert(a->parent != a);
                assert(b->parent != b);
                assert(b->parent != a);
                assert(a->parent != b);
                assert(a->count < a->parent->count);
                assert(b->count < b->parent->count);

                assert(a->parent->count == a->parent->node[0]->count + a->parent->node[1]->count);
                assert(b->parent->count == b->parent->node[0]->count + b->parent->node[1]->count);

                if (a->parent == b->parent)
                {
                    assert((a->parent->node[0] == a && a->parent->node[1] == b) ||
                           (a->parent->node[0] == b && a->parent->node[1] == a));
                    std::swap(a->parent->node[0], a->parent->node[1]);
                }
                else
                {
                    assert(a->parent->node[0] == a || a->parent->node[1] == a);
                    assert(b->parent->node[0] == b || b->parent->node[1] == b);
                    a->parent->node[a->parent->node[0] == a ? 0 : 1] = b;
                    b->parent->node[b->parent->node[0] == b ? 0 : 1] = a;
                    if (a->count != b->count)
                    {
                        UpdateCount(a->parent);
                        UpdateCount(b->parent);
                    }
                    std::swap(a->parent, b->parent);
                }

                assert(a->parent->count == a->parent->node[0]->count + a->parent->node[1]->count);
                assert(b->parent->count == b->parent->node[0]->count + b->parent->node[1]->count);

                nodes[ai] = b;
                nodes[bi] = a;

                b->index = ai;
                a->index = bi;

                UpdateSubtree(a, tick);
                UpdateSubtree(b, tick);
            };
            // increase count for branch
            static inline void IncreaseNodeCount(Node* node)
            {
                do
                {
                    node->count++;
                    node = node->parent;
                }
                while(node);
            }
            // swap nodes if needed
            static inline void SwapNodesIfNeeded(Node* node, Nodes& nodes, Node* tree)
            {
                uint64_t tick = tree->tick + 1;
                while (node->parent)
                {
                    unsigned int index = node->index;
                    while (index > 1 && nodes[index]->count > nodes[index - 1]->count)
                    {
                        LocalFunctions::SwapNodes(index - 1, index, nodes, tick);
                        index--;
                    }
                    node = node->parent;
                }
            }
        };
        // only update when there is a neighbor change detected
        m_keys[key].count++;
        if (forceUpdate)
        {
            BuildTree();
        }
        else
        {
            Node* node = m_keys[key].node;
            LocalFunctions::IncreaseNodeCount(node);
            LocalFunctions::SwapNodesIfNeeded(node,m_nodes,m_tree);
        }
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
        // fill index
        for (unsigned int i = 0; i<m_nodes.size(); ++i)
        {
            m_nodes[i]->tick = (uint64_t)-1;
            m_nodes[i]->index = i;
        }
        assert(m_tree->parent == nullptr);
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

