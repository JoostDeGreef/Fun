#include <algorithm>
#include <array>
#include <cassert>
#include <deque>

#include "StaticBlockHuffman.h"

const double StaticBlockHuffmanCommon::diffTrigger = 0.8;

StaticBlockHuffmanCommon::StaticBlockHuffmanCommon()
    : HuffmanCommon()
    , m_buffer()
{
    BuildTree();
}

StaticBlockHuffmanCompressor::StaticBlockHuffmanCompressor()
    : StaticBlockHuffmanCommon()
{
    ClearCounts(m_newCounts);
}

void StaticBlockHuffmanCompressor::WriteKeyUsingTree(unsigned int key)
{
    m_buffer.Push(m_keys[key].bits, static_cast<unsigned int>(m_keys[key].length));
}


void StaticBlockHuffmanCompressor::WriteTree()
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
    Helper(m_buffer).WriteNode(m_tree);
}

void StaticBlockHuffmanCompressor::Compress(std::vector<unsigned char>& ioBuffer)
{
    for (size_t i = 0; i < ioBuffer.size(); ++i)
    {
        const auto c = ioBuffer[i];
        ++m_newCounts[c];
        m_inBuffer.emplace_back(c);
        if (m_inBuffer.size() % blockSize == 0)
        {
            if (m_inBuffer.size() < initialBlocks * blockSize)
            {
                // do nothing
            }
            else if (m_inBuffer.size() == initialBlocks * blockSize)
            {
                m_newCounts.swap(m_counts);
            }
            else
            {
                // if the distribution in newcounts differs too much from counts, store counts and process the bytes read for that
                const double ratioCount = 1.0/(m_inBuffer.size() - blockSize);
                const double ratioNewCount = 1.0/blockSize;
                double diff = 0;
                for (size_t j = 0; j < 256; ++j)
                {
                    diff += abs(m_counts[j] * ratioCount - m_newCounts[j] * ratioNewCount);
                }
                if (diff < diffTrigger && m_inBuffer.size() <= maxBlocks * blockSize)
                {
                    for (size_t j = 0; j < 256; ++j)
                    {
                        m_counts[j] += m_newCounts[j];
                    }
                    ClearCounts(m_newCounts);
                }
                else
                {
                    WriteKeyUsingTree(keyTable);
                    BuildTree();
                    WriteTree();
                    m_counts = m_newCounts;
                    ClearCounts(m_newCounts);
                    while (m_inBuffer.size() > blockSize)
                    {
                        WriteKeyUsingTree(m_inBuffer.front());
                        m_inBuffer.pop_front();
                    }
                }
            }
        }
    }
    ioBuffer.clear();
    m_buffer.RetrieveFrontBytes(ioBuffer);        
}

void StaticBlockHuffmanCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    Compress(ioBuffer);

    if (!m_inBuffer.empty())
    {
        for (size_t i = 0; i < 256; ++i)
        {
            m_counts[i] += m_newCounts[i];
        }
        WriteKeyUsingTree(keyTable);
        BuildTree();
        WriteTree();
        for(const auto c: m_inBuffer)
        {
            WriteKeyUsingTree(c);
        }
    }

    WriteKeyUsingTree(keyEnd);

    m_buffer.FlushBack();
    if (ioBuffer.empty())
    {
        m_buffer.RetrieveFrontBytes(ioBuffer);
    }
    else
    {
        std::vector<unsigned char> temp;
        m_buffer.RetrieveFrontBytes(temp);
        ioBuffer.insert(ioBuffer.end(), temp.begin(), temp.end());
    }
}

StaticBlockHuffmanDeCompressor::StaticBlockHuffmanDeCompressor()
    : StaticBlockHuffmanCommon()
    , m_currentNode(&m_tree)
{
}


bool StaticBlockHuffmanDeCompressor::ReadTree()
{
    class Helper
    {
    public:
        Helper(BitBuffer& buffer,
               TreeCache& treeCache,
               Keys& keys)
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
        TreeCache& m_treeCache;
        Keys& m_keys;
    };
    if (m_buffer.BitsAvailable() >= m_keyCount * 2)
    {
        Helper helper(m_buffer, m_treeCache, m_keys);
        if (nullptr != helper.ReadNode())
        {
            helper.ClearKeys();
            helper.BuildKeys();
            return true;
        }
    }
    else
    {
        BitBuffer tempBuffer(m_buffer);
        Helper helper(tempBuffer, m_treeCache, m_keys);
        if (nullptr != helper.ReadNode())
        {
            helper.ClearKeys();
            helper.BuildKeys();
            tempBuffer.Swap(m_buffer);
            return true;
        }
    }
    return false;
}

void StaticBlockHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
    m_buffer.Push(ioBuffer,static_cast<unsigned int>(ioBuffer.size()*8));
    ioBuffer.clear();
    bool run = true;
    while (run)
    {
        if (m_currentNode->type == NodeType::branch)
        {
            unsigned int index;
            if(m_buffer.BitsAvailable()>=m_keys[keyEnd].length)
            {
                for(;m_currentNode->type != NodeType::leaf;)
                {
                    index = m_buffer.Pop(1u);
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
            case keyTable:
                run = run && ReadTree();
                if (run)
                {
                    m_currentNode = &m_tree;
                }
                break;
            case keyEnd:
                // see if the filling bits are 0
                if(m_buffer.BitsAvailable()<8)
                {
                    unsigned int i=m_buffer.Pop(static_cast<unsigned int>(m_buffer.BitsAvailable()));
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

void StaticBlockHuffmanDeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
    if (m_buffer.HasData() ||
        m_currentNode->type == NodeType::branch ||
        m_currentNode->leaf != keyEnd)
    {
        throw std::runtime_error("Incomplete data");
    }
}
