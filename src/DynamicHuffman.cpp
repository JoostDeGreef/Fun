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
        if (m_buffer.TryPop(index, m_minKeyLength))
        {
            m_currentNode = m_startNodes[index];
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
    m_minKeyLength = 0;
    m_startNodes.clear();
    Nodes nodes0, nodes1;
    std::vector<unsigned int> bits0, bits1;
    nodes0.reserve(32);
    nodes1.reserve(32);
    bits0.reserve(32);
    bits1.reserve(32);
    nodes0.emplace_back(m_tree);
    bits0.emplace_back(0);
    bool run = true;
    while (run)
    {
        nodes1.resize(nodes0.size() * 2);
        bits1.resize(nodes0.size() * 2);
        for (unsigned int i=0;i<nodes0.size();++i)
        {
            Node* node = nodes0[i];
            unsigned int bits = bits0[i];
            run = run && (node->node[0]->type != NodeType::leaf) && (node->node[1]->type != NodeType::leaf);
            nodes1[i*2 + 0] = node->node[0];
            nodes1[i*2 + 1] = node->node[1];
            bits1[i*2 + 0] = bits;
            bits1[i * 2 + 1] = bits | 1 << m_minKeyLength;
        }
        m_minKeyLength++;
        nodes1.swap(nodes0);
        nodes1.clear();
        bits1.swap(bits0);
        bits1.clear();
    }
    m_startNodes.resize(nodes0.size());
    for (unsigned int i = 0; i < nodes0.size(); ++i)
    {
        Node* node = nodes0[i];
        unsigned int bits = bits0[i];
        m_startNodes[bits] = node;
    }


    //m_minKeyLength = 1;
    //m_startNodes.clear();
    //m_startNodes.emplace_back(m_tree->node[0]);
    //m_startNodes.emplace_back(m_tree->node[1]);
}
