#include <algorithm>
#include <array>
#include <cassert>
#include <deque>

#include "StaticHuffman.h"

const double StaticHuffmanCommon::diffTrigger = 0.1;

StaticHuffmanCommon::StaticHuffmanCommon()
    : m_counts()
    , m_buffer()
    , m_tree(m_treeCache[0])
{
    m_counts.fill(0);
    BuildTree();
}

void StaticHuffmanCommon::BuildTree()
{
    for (unsigned int i=0;i<256;++i)
    {
        m_keys[i].value = i;
        m_keys[i].length = m_counts[i];
        m_keys[i].bits.clear();
    }
    for (unsigned int i : {keyTable, keyEnd})
    {
        m_keys[i].value = i;
        m_keys[i].length = 1;
        m_keys[i].bits.clear();
    }
    std::array<Key*, 256 + 2> keyPtr;
    for (size_t i = 0; i < 256 + 2; ++i)
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
        BiSector(std::array<Node, (256 + 2) * 2>& treeCache)
            : m_treeCache(treeCache)
            , m_cacheUsed(0)
        {}

        void Run(std::array<Key*, 256 + 2>::iterator begin,
                 std::array<Key*, 256 + 2>::iterator end)
        {
            Node* node = GetNode();
            Split(begin, end, node, BitBuffer());
        };

    private:
        std::array<Key*, 256 + 2>::iterator FindFirstAboveValue(
                   std::array<Key*, 256 + 2>::iterator begin,
                   std::array<Key*, 256 + 2>::iterator end,
                   size_t value)
        {
            auto dist = std::distance(begin, end);
            assert(dist >= 2);
            if (dist == 2)
            {
                return begin+1;
            }
            else if(dist<5)
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
                      std::array<Key*, 256 + 2>::iterator begin,
                      std::array<Key*, 256 + 2>::iterator end,
                      BitBuffer bits)
        {
            node->node[index] = GetNode();
            bits.Push(index,1);
            Split(begin,end,node->node[index],bits);
        }
        
        void Split(std::array<Key*, 256 + 2>::iterator begin,
                   std::array<Key*, 256 + 2>::iterator end,
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
                FillNode(node,0,begin,begin+1,bits);
                FillNode(node,1,begin+1,end,bits);
                break;
            default:
                {
                    node->type = NodeType::branch;
                    size_t avg = ((**begin).length + (**(begin + dist - 1)).length) / 2;
                    auto mid = FindFirstAboveValue(begin,end,avg); 
                    FillNode(node,0,begin,mid,bits);
                    FillNode(node,1,mid,end,bits);
                    break;
                }
            }
        };

        Node* GetNode()
        {
            return &m_treeCache[m_cacheUsed++];
        }

        std::array<Node, (256 + 2) * 2>& m_treeCache;
        unsigned int m_cacheUsed;
    };
    BiSector bisector(m_treeCache);
    bisector.Run(keyPtr.begin(), keyPtr.begin() + filled);
}

StaticHuffmanCompressor::StaticHuffmanCompressor()
    : StaticHuffmanCommon()
{
    m_newCounts.fill(0);
}

void StaticHuffmanCompressor::WriteKeyUsingTree(unsigned int key)
{
    m_buffer.Push(m_keys[key].bits, static_cast<unsigned int>(m_keys[key].length));
}


void StaticHuffmanCompressor::WriteTree()
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

void StaticHuffmanCompressor::Compress(std::vector<unsigned char>& ioBuffer)
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
                for (size_t i = 0; i < 256; ++i)
                {
                    diff += abs(m_counts[i] * ratioCount - m_newCounts[i] * ratioNewCount);
                }
                if (diff < diffTrigger)
                {
                    for (size_t i = 0; i < 256; ++i)
                    {
                        m_counts[i] += m_newCounts[i];
                    }
                    m_newCounts.fill(0);
                }
                else
                {
                    WriteKeyUsingTree(keyTable);
                    BuildTree();
                    WriteTree();
                    m_counts = m_newCounts;
                    m_newCounts.fill(0);
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

void StaticHuffmanCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    Compress(ioBuffer);

    if (!m_inBuffer.empty())
    {
        m_counts = m_newCounts;
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

StaticHuffmanDeCompressor::StaticHuffmanDeCompressor()
    : StaticHuffmanCommon()
    , m_currentNode(&m_tree)
{
}


bool StaticHuffmanDeCompressor::ReadTree()
{
    class Helper
    {
    public:
        Helper(BitBuffer& buffer,
               std::array<Node, (256 + 2) * 2>& treeCache,
               std::array<Key, 256 + 2>& keys)
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
        std::array<Node, (256 + 2) * 2>& m_treeCache;
        std::array<Key, 256 + 2>& m_keys;
    };
    BitBuffer tempBuffer(m_buffer);
    Helper helper(tempBuffer,m_treeCache,m_keys);
    if (nullptr != helper.ReadNode())
    {
        helper.ClearKeys();
        helper.BuildKeys();
        tempBuffer.Swap(m_buffer);
        return true;
    }
    else
    {
        return false;
    }
}

void StaticHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
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
                    unsigned int i=m_buffer.Pop(m_buffer.BitsAvailable());
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
    if (m_buffer.HasData() ||
        m_currentNode->type == NodeType::branch ||
        m_currentNode->leaf != keyEnd)
    {
        throw std::runtime_error("Incomplete data");
    }
}
