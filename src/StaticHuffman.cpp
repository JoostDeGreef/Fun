#include <algorithm>
#include <array>
#include <cassert>
#include <deque>

#include "StaticHuffman.h"

StaticHuffmanCommon::StaticHuffmanCommon()
    : HuffmanCommon()
{
}

StaticHuffmanCompressor::StaticHuffmanCompressor()
    : StaticHuffmanCommon()
{
}

void StaticHuffmanCompressor::WriteKeyUsingTree(BitBuffer& buffer, unsigned int key) const
{
    buffer.Push(m_keys[key].bits, static_cast<unsigned int>(m_keys[key].length));
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
                m_buffer.Push(node.leaf, 9u);
            }
        }
    private:
        BitBuffer& m_buffer;
    };
    Helper(buffer).WriteNode(m_tree);
}

void StaticHuffmanCompressor::Compress(std::vector<unsigned char>& ioBuffer)
{
    for (size_t i = 0; i < ioBuffer.size(); ++i)
    {
        const auto c = ioBuffer[i];
        ++m_counts[c];
        m_inBuffer.emplace_back(c);
    }
    ioBuffer.clear();
}

void StaticHuffmanCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    BitBuffer buffer;

    Compress(ioBuffer);

    BuildTree();
    WriteTree(buffer);

    if (!m_inBuffer.empty())
    {
        for(const auto c: m_inBuffer)
        {
            WriteKeyUsingTree(buffer,c);
        }
    }

    WriteKeyUsingTree(buffer,keyEnd);

    buffer.FlushBack();
    buffer.RetrieveFrontBytes(ioBuffer);
}



StaticHuffmanDeCompressor::StaticHuffmanDeCompressor()
    : StaticHuffmanCommon()
    , m_currentNode(nullptr)
    , m_inBuffer()
{
}


bool StaticHuffmanDeCompressor::ReadTree()
{
    class Helper
    {
    public:
        Helper(BitBuffer& buffer,
               std::array<Node, (256 + 1) * 2>& treeCache,
               std::array<Key, 256 + 1>& keys)
            : m_index(0)
            , m_buffer(buffer)
            , m_treeCache(treeCache)
            , m_keys(keys)
        {}

        Node* ReadNode()
        {
            if (m_buffer.BitsAvailable() < 10)
            {
                return nullptr;
            }
            Node& node = m_treeCache[m_index];
            ++m_index;
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
                node.leaf = m_buffer.Pop(9u);
            }
            return &node;
        }

        void ClearKeys()
        {
            unsigned int value = 0;
            for (auto& key : m_keys)
            {
                key.bits.clear();
                key.value = value++;
                key.length = 0;
            }
        }

        void BuildKeys()
        {
            AddNodeToKey(m_treeCache[0].node[0], 0, BitBuffer());
            AddNodeToKey(m_treeCache[0].node[1], 1, BitBuffer());
        }
    private:
        void AddNodeToKey(Node* node, unsigned int index, BitBuffer bits)
        {
            bits.Push(index, 1u);
            if (node->type == NodeType::branch)
            {
                AddNodeToKey(node->node[0], 0, BitBuffer());
                AddNodeToKey(node->node[1], 1, BitBuffer());
            }
            else
            {
                m_keys[node->leaf].length = bits.BitsAvailable();
                bits.FlushBack();
                bits.RetrieveFrontBytes(m_keys[node->leaf].bits);
            }
        }
        unsigned int m_index = 0;
        BitBuffer& m_buffer;
        std::array<Node, (256 + 1) * 2>& m_treeCache;
        std::array<Key, 256 + 1>& m_keys;
    };
    if (m_inBuffer.BitsAvailable() >= (256 + 1) * 2)
    {
        Helper helper(m_inBuffer, m_treeCache, m_keys);
        if (nullptr != helper.ReadNode())
        {
            helper.ClearKeys();
            helper.BuildKeys();
            return true;
        }
    }
    else
    {
        BitBuffer tempBuffer(m_inBuffer);
        Helper helper(tempBuffer, m_treeCache, m_keys);
        if (nullptr != helper.ReadNode())
        {
            helper.ClearKeys();
            helper.BuildKeys();
            tempBuffer.Swap(m_inBuffer);
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
            m_currentNode = &m_tree;
        }
    }
    while (run)
    {
        if (m_currentNode->type == NodeType::branch)
        {
            unsigned int index;
            if(m_inBuffer.BitsAvailable()>=m_keys[keyEnd].length)
            {
                for(;m_currentNode->type != NodeType::leaf;)
                {
                    index = m_inBuffer.Pop(1u);
                    m_currentNode = m_currentNode->node[index];
                }
            }
            else
            {
                run = false;
            }
        }
        else
        {
            switch (m_currentNode->leaf)
            {
            case keyEnd:
                // see if the filling bits are 0
                if(m_inBuffer.BitsAvailable()<8)
                {
                    unsigned int i=m_inBuffer.Pop(static_cast<unsigned int>(m_inBuffer.BitsAvailable()));
                    if(i != 0)
                    {
                        throw std::runtime_error("Invalid data");
                    }
                }
                // stop any further actions.
                run = false;
                break;
            default:
                ioBuffer.emplace_back(static_cast<unsigned char>(m_currentNode->leaf));
                m_currentNode = &m_tree;
                break;
            }
        }
    }
}

void StaticHuffmanDeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
    if (m_inBuffer.HasData() ||
        m_currentNode->type == NodeType::branch ||
        m_currentNode->leaf != keyEnd)
    {
        throw std::runtime_error("Incomplete data");
    }
}
