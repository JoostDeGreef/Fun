#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <string>

#include "DynamicHuffman2.h"

void DynamicHuffman2Common::BuildTree()
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
    auto SortKeys = [](Node* a, Node* b)
    {
        assert(a->type == NodeType::leaf);
        assert(b->type == NodeType::leaf);
        return (a->count > b->count) || (a->count == b->count && a->key->value < b->key->value);
    };
    std::sort(m_nodes.begin(), m_nodes.end(), SortKeys);
    // repeatedly join the 2 least important nodes until there is one node left
    auto CompareNodes = [](Node* a, Node* b)
    {
        return a->count >= b->count;
    };
    Node* node = nullptr;
    for (size_t nodeCount = m_nodes.size(); nodeCount >= 2; --nodeCount)
    {
        assert(m_nodes.size() < m_nodeCache.size());
        node = &m_nodeCache[m_nodes.size()];
        node->SetNodes(m_nodes[nodeCount - 2], m_nodes[nodeCount - 1]);
        auto iter = std::lower_bound(m_nodes.begin(), m_nodes.begin() + nodeCount - 2, node,CompareNodes);
        m_nodes.emplace(iter, node);
    }
    m_tree = node;
    // fill before/after
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
    // todo: make this optional, only used by decompress
    // fill all bits
    class Fill
    {
    public:
        static void Bits(Node* node)
        {
            KeyBits bits;
            DiveIntoNode(node->node[0], 0, bits);
            DiveIntoNode(node->node[1], 1, bits);
        }
    private:
        static void DiveIntoNode(Node* node, unsigned int index, KeyBits bits)
        {
            bits.Push(index, 1u);
            if (node->type == NodeType::branch)
            {
                DiveIntoNode(node->node[0], 0, bits);
                DiveIntoNode(node->node[1], 1, bits);
            }
            else
            {
                node->key->bits = bits;
            }
        }
    };
    Fill::Bits(m_tree);
}

void DynamicHuffman2Common::UpdateHistory(unsigned int key)
{
// todo: only update when there is a neighbor change detected
    m_keys[key].count++;
    std::swap(m_history[m_historyIndex],key);
    if (key != static_cast<unsigned int>(-1))
    {
        m_keys[key].count--;
        if (m_keys[key].count == 0)
        {
            m_keys[keyNew].count--;
        }
    }
    m_historyIndex++;
    if (m_historyIndex >= lifetime)
    {
        m_historyIndex = 0;
    }
    BuildTree();
}


DynamicHuffman2Compressor::DynamicHuffman2Compressor()
    : DynamicHuffman2Common()
{}

void DynamicHuffman2Compressor::WriteKeyUsingTree(unsigned int key)
{
    Key& k = m_keys[key];
    if (k.bits.Empty())
    {
        k.FillBits();
    }
    m_buffer.Push(k.bits);
}

void DynamicHuffman2Compressor::Compress(std::vector<unsigned char>& ioBuffer)
{
    for (size_t i = 0; i < ioBuffer.size(); ++i)
    {
        const auto c = ioBuffer[i];
        if (m_keys[c].count == 0)
        {
            WriteKeyUsingTree(keyNew);
            m_buffer.Push(c, 8u);
            m_keys[keyNew].count++;
        }
        else
        {
            WriteKeyUsingTree(c);
        }
        UpdateHistory(c);
    }
    ioBuffer.clear();
    m_buffer.RetrieveFrontBytes(ioBuffer);        
}

void DynamicHuffman2Compressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    Compress(ioBuffer);

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

DynamicHuffman2DeCompressor::DynamicHuffman2DeCompressor()
    : DynamicHuffman2Common()
    , m_currentNode(m_tree)
{
}

void DynamicHuffman2DeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
    m_buffer.Push(ioBuffer,static_cast<unsigned int>(ioBuffer.size()*8));
    ioBuffer.clear();
    bool run = true;
    while (run)
    {
        if (m_currentNode->type == NodeType::branch)
        {
            unsigned int index;
            if(m_buffer.BitsAvailable()>=m_keys[keyEnd].bits.Length())
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
            switch (m_currentNode->key->value)
            {
            case keyNew:
                run = run && m_buffer.BitsAvailable() >= 8;
                if (run)
                {
                    unsigned int c = m_buffer.Pop(8u);
                    ioBuffer.emplace_back(static_cast<unsigned char>(c));
                    assert(m_keys[c].count == 0);
                    m_keys[keyNew].count++;
                    UpdateHistory(c);
                    m_currentNode = m_tree;
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
                ioBuffer.emplace_back(static_cast<unsigned char>(m_currentNode->key->value));
                assert(m_keys[m_currentNode->key->value].count != 0);
                UpdateHistory(m_currentNode->key->value);
                m_currentNode = m_tree;
                break;
            }
        }
    }
}

void DynamicHuffman2DeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
    if (m_buffer.HasData() ||
        m_currentNode->type == NodeType::branch ||
        m_currentNode->key->value != keyEnd)
    {
        throw std::runtime_error("Incomplete data");
    }
}
