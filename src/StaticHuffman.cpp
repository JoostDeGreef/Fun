#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <string>

#include "StaticHuffman.h"

StaticHuffmanCompressor::StaticHuffmanCompressor()
{
    m_nodes.reserve(keyCount);
}

void StaticHuffmanCompressor::ClearKeys()
{
    for (unsigned int i = 0; i < 256; ++i)
    {
        m_keys[i].count = 0;
    }
    m_keys[keyEnd].count = 1;
}

void StaticHuffmanCompressor::BuildTree()
{
    m_nodes.clear();
    // add nodes for all used keys
    for (unsigned int i = 0; i < keyCount; ++i)
    {
        if (m_keys[i].count > 0)
        {
            m_keys[i].bits.Clear();
            m_nodes.emplace_back(&m_nodeCache[m_nodes.size()]);
            m_nodes.back()->key = i;
            m_nodes.back()->type = NodeType::leaf;
        }
    }
    // sort nodes, these are all 'key nodes'
    std::sort(m_nodes.begin(), m_nodes.end(), [](Node* a, Node* b)
    {
        assert(a->type == NodeType::leaf);
        assert(b->type == NodeType::leaf);
        return (a->count > b->count) || (a->count == b->count && a->key < b->key);
    });
    // repeatedly join the 2 least important nodes until there is one node left
    Node* node = nullptr;
    for (size_t nodeCount = m_nodes.size(); nodeCount >= 2; --nodeCount)
    {
        assert(m_nodes.size() < m_nodeCache.size());
        node = &m_nodeCache[m_nodes.size()];
        node->type = NodeType::branch;
        node->node[0] = m_nodes[nodeCount - 2];
        node->node[1] = m_nodes[nodeCount - 1];
        auto iter = std::lower_bound(m_nodes.begin(), m_nodes.begin() + nodeCount - 2, node, [](Node* a, Node* b)
        {
            return (a->count > b->count);
        });
        m_nodes.emplace(iter, node);
    }
    m_tree = node;
    // fill bits
    struct LocalFunctions
    {
        static void FillBits(Keys& keys, Node* node, unsigned int index, BitBuffer bits)
        {
            bits.Push(index, 1u);
            if (node->type == NodeType::leaf)
            {
                keys[node->key].bits.Swap(bits);
            }
            else
            {
                FillBits(keys, node->node[0], 0, bits);
                FillBits(keys, node->node[1], 1, bits);
            }
        }
    };
    BitBuffer bits;
    LocalFunctions::FillBits(m_keys, m_tree->node[0], 0, bits);
    LocalFunctions::FillBits(m_keys, m_tree->node[1], 1, bits);
}

void StaticHuffmanCompressor::WriteTree(BitBuffer& buffer) const
{
    class Helper
    {
    public:
        Helper(BitBuffer& buffer)
            : m_buffer(buffer)
        {}

        void WriteNode(const Node& node)
        {
            if (node.type == NodeType::branch)
            {
                m_buffer.Push(0u, 1u);
                WriteNode(*node.node[0]);
                WriteNode(*node.node[1]);
            }
            else
            {
                m_buffer.Push(1u, 1u);
                m_buffer.Push(node.key, 9u);
            }
        }
    private:
        BitBuffer& m_buffer;
    };
    Helper(buffer).WriteNode(*m_tree);
}

void StaticHuffmanCompressor::WriteKeyUsingTree(BitBuffer& buffer, unsigned int key) const
{
    buffer.Push(m_keys[key].bits);
}

void StaticHuffmanCompressor::CompressBuffer(std::vector<unsigned char>::const_iterator begin, std::vector<unsigned char>::const_iterator end)
{
    ClearKeys();
    for (auto iter = begin; iter!=end; ++iter)
    {
        ++m_keys[*iter].count;
    }
    BuildTree();
    WriteTree(m_outBuffer);
    for (auto iter = begin; iter != end; ++iter)
    {
        WriteKeyUsingTree(m_outBuffer,*iter);
    }
}

void StaticHuffmanCompressor::Compress(std::vector<unsigned char>& ioBuffer)
{
    std::vector<unsigned char>::iterator begin;
    std::vector<unsigned char>::iterator end;
    if (!m_inBuffer.empty())
    {
        if (m_inBuffer.size() + ioBuffer.size() < blockSize)
        {
            m_inBuffer.insert(m_inBuffer.end(), ioBuffer.begin(), ioBuffer.end());
            begin = end = std::end(ioBuffer);
        }
        else
        {
            begin = std::begin(ioBuffer);
            end = begin + blockSize - m_inBuffer.size();
            m_inBuffer.insert(m_inBuffer.end(), begin, end);
            CompressBuffer(m_inBuffer.begin(), m_inBuffer.end());
            m_inBuffer.clear();
        }
    }
    else
    {
        begin = end = std::begin(ioBuffer);
    }
    while (std::distance(end,ioBuffer.end()) >= blockSize)
    {
        begin = end;
        end += blockSize;
        CompressBuffer(begin, end);
    }
    if (end != ioBuffer.end())
    {
        m_inBuffer.insert(m_inBuffer.end(), end, ioBuffer.end());
    }
    ioBuffer.clear();
    m_outBuffer.RetrieveFrontBytes(ioBuffer);
}

void StaticHuffmanCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    Compress(ioBuffer);

    if (!m_inBuffer.empty())
    {
        CompressBuffer(m_inBuffer.cbegin(), m_inBuffer.cend());
    }

    WriteKeyUsingTree(m_outBuffer,keyEnd);
    m_outBuffer.FlushBack();
    m_outBuffer.RetrieveFrontBytes(ioBuffer);
}



StaticHuffmanDeCompressor::StaticHuffmanDeCompressor()
    : m_currentNode(nullptr)
    , m_inBuffer()
    , m_blockCount(0)
{
}


bool StaticHuffmanDeCompressor::ReadTree()
{
    class Helper
    {
    public:
        Helper(BitBuffer& buffer,
               NodeCache& nodeCache)
            : m_index(0)
            , m_buffer(buffer)
            , m_nodeCache(nodeCache)
        {}

        Node* ReadTree()
        {
            return ReadNode();
        }
    private:
        Node* ReadNode()
        {
            if (m_buffer.BitsAvailable() < 10)
            {
                return nullptr;
            }
            Node& node = m_nodeCache[m_index];
            ++m_index;
            node.count = 0;
            node.type = m_buffer.Pop(1u) == 0 ? NodeType::branch : NodeType::leaf;
            if (node.type == NodeType::branch)
            {
                if ((node.node[0] = ReadNode()) == nullptr ||
                    (node.node[1] = ReadNode()) == nullptr)
                {
                    return nullptr;
                }
            }
            else
            {
                node.key = m_buffer.Pop(9u);
            }
            return &node;
        }

    private:
        unsigned int m_index = 0;
        BitBuffer& m_buffer;
        NodeCache& m_nodeCache;
    };
    if (m_inBuffer.BitsAvailable() >= keyCount * 2)
    {
        Helper helper(m_inBuffer, m_nodeCache);
        if (nullptr != (m_tree = helper.ReadTree()))
        {
            FillStartNodes();
            return true;
        }
    }
    else
    {
        BitBuffer tempBuffer(m_inBuffer);
        Helper helper(tempBuffer, m_nodeCache);
        if (nullptr != (m_tree = helper.ReadTree()))
        {
            tempBuffer.Swap(m_inBuffer);
            FillStartNodes();
            return true;
        }
    }
    return false;
}

void StaticHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
    m_inBuffer.Push(ioBuffer,static_cast<unsigned int>(ioBuffer.size()*8));
    ioBuffer.clear();
    bool run = true;
    if (m_currentNode == nullptr)
    {
        run = run && ReadTree();
        if (run)
        {
            m_currentNode = m_tree;
            assert(nullptr != m_currentNode);
        }
    }
    while (run)
    {
        if (m_currentNode->type == NodeType::branch)
        {
            unsigned int index;
            while(run && m_currentNode->type != NodeType::leaf)
            {
                run = m_inBuffer.TryPop(index,1u);
                if (run)
                {
                    m_currentNode = m_currentNode->node[index];
                }
            }
        }
        else
        {
            switch (m_currentNode->key)
            {
            case keyEnd:
                // see if the filling bits are 0
                if (m_inBuffer.BitsAvailable() < 8)
                {
                    unsigned int i = m_inBuffer.Pop(static_cast<unsigned int>(m_inBuffer.BitsAvailable()));
                    if (i != 0)
                    {
                        throw std::runtime_error("Invalid data");
                    }
                }
                else
                {
                    throw std::runtime_error("Data after end");
                }
                // stop any further actions.
                run = false;
                break;
            default:
                ioBuffer.emplace_back(static_cast<unsigned char>(m_currentNode->key));
                m_blockCount++;
                if (m_blockCount >= blockSize)
                {
                    m_currentNode = nullptr;
                    m_blockCount = 0;
                    run = run && ReadTree();
                }
                if (run)
                {
                    unsigned int index;
                    if (m_inBuffer.TryPeek(index, startNodeBits))
                    {
                        m_currentNode = m_startNodes[index];
                        m_inBuffer.Pop(m_currentNode->depth);
                    }
                    else
                    {
                        m_currentNode = m_tree;
                    }
                }
                else
                {
                    m_currentNode = m_tree;
                }
                break;
            }
        }
    }
}

void StaticHuffmanDeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
    if (m_inBuffer.HasData() ||
        (m_currentNode != nullptr && (m_currentNode->type == NodeType::branch || m_currentNode->key != keyEnd)))
    {
        throw std::runtime_error("Incomplete data");
    }
}

void StaticHuffmanDeCompressor::FillStartNodes()
{
    // todo: turn this into a recursive template
    class Helper
    {
    public:
        inline
            static void Fill(Nodes& nodes, Node* tree)
        {
            FillBranch(nodes, tree->node[0], 0, 1);
            FillBranch(nodes, tree->node[1], 1, 1);
        }
    private:
        inline
            static void FillBranch(Nodes& nodes, Node* node, const unsigned int index, const unsigned int depth)
        {
            if (depth < startNodeBits)
            {
                if (node->type == NodeType::branch)
                {
                    FillBranch(nodes, node->node[0], index, depth + 1);
                    FillBranch(nodes, node->node[1], index | 1 << depth, depth + 1);
                }
                else
                {
                    node->depth = depth;
                    FillLeaf(nodes, node, index, depth + 1);
                    FillLeaf(nodes, node, index | 1 << depth, depth + 1);
                }
            }
            else
            {
                node->depth = depth;
                nodes[index] = node;
            }
        }
        inline
            static void FillLeaf(Nodes& nodes, Node* node, const unsigned int index, const unsigned int depth)
        {
            if (depth < startNodeBits)
            {
                FillLeaf(nodes, node, index, depth + 1);
                FillLeaf(nodes, node, index | 1 << depth, depth + 1);
            }
            else
            {
                nodes[index] = node;
            }
        }
    };
    m_startNodes.resize(1 << startNodeBits, nullptr);
    Helper::Fill(m_startNodes, m_tree);
}

