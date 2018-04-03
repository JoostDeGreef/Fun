#include <algorithm>
#include <array>
#include <cassert>
#include <deque>

#include "DynamicHuffman.h"

DynamicHuffmanCommon::DynamicHuffmanCommon()
    : HuffmanCommon()
    , m_buffer()
    , m_historyIndex(0)
{
    BuildTree();
    m_history.fill(-1);
}

void DynamicHuffmanCommon::UpdateHistory(unsigned int key)
{
    m_counts[key]++;
    std::swap(m_history[m_historyIndex],key);
    if (key != static_cast<unsigned int>(-1))
    {
        m_counts[key]--;
    }
    m_historyIndex++;
    if (m_historyIndex >= lifetime)
    {
        m_historyIndex = 0;
    }
    BuildTree();
}


DynamicHuffmanCompressor::DynamicHuffmanCompressor()
    : DynamicHuffmanCommon()
{}

void DynamicHuffmanCompressor::WriteKeyUsingTree(unsigned int key)
{
    m_buffer.Push(m_keys[key].bits, static_cast<unsigned int>(m_keys[key].length));
}

// todo: also 'count' keyNew
void DynamicHuffmanCompressor::Compress(std::vector<unsigned char>& ioBuffer)
{
    for (size_t i = 0; i < ioBuffer.size(); ++i)
    {
        const auto c = ioBuffer[i];
        if (m_counts[c] == 0)
        {
            WriteKeyUsingTree(keyNew);
            m_buffer.Push(c, 8u);
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
    , m_currentNode(&m_tree)
{
}

void DynamicHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
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
            case keyNew:
                run = run && m_buffer.BitsAvailable() >= 8;
                if (run)
                {
                    unsigned int c = m_buffer.Pop(8u);
                    ioBuffer.emplace_back(static_cast<unsigned char>(c));
                    assert(m_counts[c] == 0);
                    UpdateHistory(c);
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
                assert(m_counts[m_currentNode->leaf] != 0);
                UpdateHistory(m_currentNode->leaf);
                m_currentNode = &m_tree;
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
        m_currentNode->leaf != keyEnd)
    {
        throw std::runtime_error("Incomplete data");
    }
}
