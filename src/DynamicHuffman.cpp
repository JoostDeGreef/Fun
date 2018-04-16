#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <string>

#include "DynamicHuffman.h"

DynamicHuffmanCompressor::DynamicHuffmanCompressor()
    : DynamicHuffmanCommon()
{}

void DynamicHuffmanCompressor::WriteKeyUsingTree(unsigned int key)
{
    Key& k = m_keys[key];
    if (k.bits.Empty())
    {
        k.FillBits();
    }
    m_buffer.Push(k.bits);
}

void DynamicHuffmanCompressor::Compress(std::vector<unsigned char>& ioBuffer)
{
    for (size_t i = 0; i < ioBuffer.size(); ++i)
    {
        const auto c = ioBuffer[i];
        if (m_keys[c].count == 0)
        {
            WriteKeyUsingTree(keyNew);
            m_buffer.Push(c, 8u);
            m_keys[keyNew].count++;
            UpdateTree(c,true);
        }
        else
        {
            WriteKeyUsingTree(c);
            UpdateTree(c,false);
        }
    }
    ioBuffer.clear();
    m_buffer.RetrieveFrontBytes(ioBuffer);
}

void DynamicHuffmanCompressor::Finish(std::vector<unsigned char>& ioBuffer)
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

DynamicHuffmanDeCompressor::DynamicHuffmanDeCompressor()
    : DynamicHuffmanCommon()
    , m_currentNode(m_tree)
{
    FillStartNodes();
}

void DynamicHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
    m_buffer.Push(ioBuffer, static_cast<unsigned int>(ioBuffer.size() * 8));
    ioBuffer.clear();
    bool run = true;
    unsigned int index;
    auto ResetTree = [&index,this](const unsigned int key, const bool forceUpdate)
    {
        if (UpdateTree(key, forceUpdate))
        {
            FillStartNodes();
        }
        if (m_buffer.TryPeek(index, startNodeBits))
        {
            m_currentNode = m_startNodes[index];
            m_buffer.Pop(m_currentNode->depth);
        }
        else
        {
            m_currentNode = m_tree;
        }
    };
    while (run)
    {
        if (m_currentNode->type == NodeType::branch)
        {
            while (run && m_currentNode->type != NodeType::leaf)
            {
                run = m_buffer.TryPop(index,1u);
                if (run)
                {
                    m_currentNode = m_currentNode->node[index];
                }
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
                    ResetTree(c,true);
                }
                break;
            case keyEnd:
                // see if the filling bits are 0
                if (m_buffer.BitsAvailable()<8)
                {
                    unsigned int i = m_buffer.Pop(static_cast<unsigned int>(m_buffer.BitsAvailable()));
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
                ioBuffer.emplace_back(static_cast<unsigned char>(m_currentNode->key->value));
                assert(m_keys[m_currentNode->key->value].count != 0);
                ResetTree(m_currentNode->key->value, false);
                break;
            }
        }
    }
}

void DynamicHuffmanDeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
    if (m_buffer.HasData() ||
        m_currentNode->type == NodeType::branch ||
        m_currentNode->key->value != keyEnd)
    {
        throw std::runtime_error("Incomplete data");
    }
}

void DynamicHuffmanDeCompressor::FillStartNodes()
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
    m_startNodes.resize(1 << startNodeBits,nullptr);
    Helper::Fill(m_startNodes, m_tree);
}

