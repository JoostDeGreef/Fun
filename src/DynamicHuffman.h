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
        Node* after;
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

    void UpdateTree(unsigned int key, const bool forceUpdate)
    {
        struct LocalFunctions
        {
            // clear the Key bits in this subtree, adjust node order for nodes which share a parent.
            static inline void UpdateSubtree(Node* node)
            {
                assert(node->parent != node);
                assert(node->before != node);
                assert(node->after != node);
                assert(node->after != node->before);
                assert(node->parent != node->after);
                if (node->type == NodeType::branch)
                {
                    UpdateSubtree(node->node[0]);
                    UpdateSubtree(node->node[1]);
                }
                else
                {
                    node->key->ClearBits();
                }
            };
            // swap 2 nodes
            static inline void SwapNodes(Node* a, Node* b)
            {
                assert(a->parent != a);
                assert(a->before != a);
                assert(a->after != a);
                assert(a->after != a->before);
                assert(a->parent != a->after);

                assert(b->parent != b);
                assert(b->before != b);
                assert(b->after != b);
                assert(b->after != b->before);
                assert(b->parent != b->after);

                assert(b->parent != a);
                assert(a->parent != b);

                std::swap(a->count, b->count);
                std::swap(a->type, b->type);
                if (a->type == NodeType::branch)
                {
                    if (b->type == NodeType::branch)
                    {
                        std::swap(a->node, b->node);
                        b->node[0]->parent = b;
                        b->node[1]->parent = b;
                    }
                    else
                    {
                        Key* key = a->key;
                        a->node[0] = b->node[0];
                        a->node[1] = b->node[1];
                        b->key = key;
                        b->key->node = b;
                    }
                    a->node[0]->parent = a;
                    a->node[1]->parent = a;
                }
                else
                {
                    if (b->type == NodeType::branch)
                    {
                        Key* key = b->key;
                        b->node[0] = a->node[0];
                        b->node[1] = a->node[1];
                        a->key = key;
                        a->key->node = a;
                        b->node[0]->parent = b;
                        b->node[1]->parent = b;
                    }
                    else
                    {
                        std::swap(a->key, b->key);
                        a->key->node = a;
                        b->key->node = b;
                    }
                }

                UpdateSubtree(a);
                UpdateSubtree(b);
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
            static inline void SwapNodesIfNeeded(Node* node)
            {
                do
                {
                    while (node->before && node->before->count < node->count)
                    {
                        LocalFunctions::SwapNodes(node->before, node);
                    }
                    node = node->parent;
                } 
                while (node && node->parent);
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
            LocalFunctions::SwapNodesIfNeeded(node);
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
        // fill before & after
        {
            Node* prev = nullptr;
            for (auto& node : m_nodes)
            {
                node->before = prev;
                prev = node;
            }
            prev->after = nullptr;
            while (prev->before)
            {
                prev->before->after = prev;
                prev = prev->before;
            }
            assert(m_tree->before == nullptr);
            assert(m_tree->parent == nullptr);
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

