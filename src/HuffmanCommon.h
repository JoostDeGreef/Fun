#pragma once

#include "BitBuffer.h"

template<size_t KEY_COUNT>
class HuffmanCommon
{
protected:
    static const size_t m_keyCount = KEY_COUNT;

    HuffmanCommon()
        : m_counts()
        , m_tree(m_treeCache[0])
    {
        ClearCounts(m_counts);
    }

    typedef std::array<size_t, m_keyCount> Counts;
    Counts m_counts;

    void ClearCounts(Counts& counts)
    {
        counts.fill(0);
        // extra keys (commands)
        for (unsigned int i = 256; i<m_keyCount; ++i)
        {
            counts[i] = 1;
        }
    }

    struct Key
    {
        unsigned int value;
        size_t length;
        std::vector<unsigned char> bits;
    };
    typedef std::array<Key*, m_keyCount> KeyPtrs;
    typedef std::array<Key, m_keyCount> Keys;
    Keys m_keys;

    enum class NodeType
    {
        branch,
        leaf
    };
    struct Node
    {
        NodeType type;
        unsigned int depth;
        union
        {
            Node* node[2];
            unsigned int leaf;
        };
    };
    typedef std::array<Node, m_keyCount * 2> TreeCache;
    TreeCache m_treeCache;
    Node& m_tree;
    typedef std::vector<Node*> Nodes;

    void BuildTree()
    {
        for (unsigned int i = 0; i<m_keyCount; ++i)
        {
            m_keys[i].value = i;
            m_keys[i].length = m_counts[i];
            m_keys[i].bits.clear();
        }
        KeyPtrs keyPtr;
        for (size_t i = 0; i < m_keyCount; ++i)
        {
            keyPtr[i] = &m_keys[i];
        }
        // sort by count,value
        auto sortByCount = [](Key* a, Key* b)
        {
            return (a->length > b->length) || (a->length == b->length && a->value < b->value);
        };
        std::sort(keyPtr.begin(), keyPtr.end(), sortByCount);
        // make counts cummulative for quick bisecting
        size_t count = 0;
        size_t filled = 0;
        for (auto& ptr : keyPtr)
        {
            if (ptr->length > 0)
            {
                ++filled;
            }
            count += ptr->length;
            ptr->length = count;
        }
        class BiSector
        {
        public:
            BiSector(TreeCache& treeCache)
                : m_treeCache(treeCache)
                , m_cacheUsed(0)
            {}

            void Run(KeyPtrs::iterator begin,KeyPtrs::iterator end)
            {
                Node* node = GetNode();
                Split(begin, end, node, BitBuffer());
            };

        private:
            KeyPtrs::iterator FindFirstAboveValue(
                KeyPtrs::iterator begin,
                KeyPtrs::iterator end,
                size_t value)
            {
                auto dist = std::distance(begin, end);
                assert(dist >= 2);
                if (dist == 2)
                {
                    return begin + 1;
                }
                else if (dist<5)
                {
                    auto mid = begin + 1;
                    auto next = mid + 1;
                    for (; next != end && (**mid).length<value; ++next)
                    {
                        mid = next;
                    }
                    return mid;
                }
                else
                {
                    auto mid = begin + (dist / 2);
                    if ((**mid).length < value)
                    {
                        mid = FindFirstAboveValue(mid, end, value);
                    }
                    else
                    {
                        mid = FindFirstAboveValue(begin, mid, value);
                    }
                    return mid;
                }
            }

            void FillNode(Node* node,
                unsigned int index,
                KeyPtrs::iterator begin,
                KeyPtrs::iterator end,
                BitBuffer bits)
            {
                node->node[index] = GetNode();
                bits.Push(index, 1);
                Split(begin, end, node->node[index], bits);
            }

            void Split(KeyPtrs::iterator begin,
                KeyPtrs::iterator end,
                Node* node,
                BitBuffer bits)
            {
                auto dist = std::distance(begin, end);
                switch (dist)
                {
                case 0:
                    assert(false);
                case 1:
                    node->type = NodeType::leaf;
                    node->leaf = (**begin).value;
                    (**begin).length = bits.BitsAvailable();
                    bits.FlushBack();
                    bits.RetrieveFrontBytes((**begin).bits);
                    break;
                case 2:
                    node->type = NodeType::branch;
                    FillNode(node, 0, begin, begin + 1, bits);
                    FillNode(node, 1, begin + 1, end, bits);
                    break;
                default:
                {
                    node->type = NodeType::branch;
                    size_t avg = ((**begin).length + (**(begin + dist - 1)).length) / 2;
                    auto mid = FindFirstAboveValue(begin, end, avg);
                    FillNode(node, 0, begin, mid, bits);
                    FillNode(node, 1, mid, end, bits);
                    break;
                }
                }
            };

            Node* GetNode()
            {
                return &m_treeCache[m_cacheUsed++];
            }

            TreeCache& m_treeCache;
            unsigned int m_cacheUsed;
        };
        BiSector bisector(m_treeCache);
        bisector.Run(keyPtr.begin(), keyPtr.begin() + filled);
    }
};

