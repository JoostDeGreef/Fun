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
    for (auto& ptr : keyPtr)
    {
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
            // todo
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
    bisector.Run(keyPtr.begin(), keyPtr.end());
}


StaticHuffmanCompressor::StaticHuffmanCompressor()
    : StaticHuffmanCommon()
{
    m_newCounts.fill(0);
}

void StaticHuffmanCompressor::WriteKeyUsingTree(unsigned int key)
{
    // todo
}

void StaticHuffmanCompressor::WriteTree()
{
    // todo
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
                    for (size_t i = 0; i < 256; ++i)
                    {
                        m_counts[i] = m_newCounts[i];
                    }
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
}


void StaticHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
}

void StaticHuffmanDeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
}
