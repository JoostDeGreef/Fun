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
    m_buffer.Reserve(ioBuffer.size() * sizeof(ioBuffer[0]) * 8);
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
    m_buffer.Pop(ioBuffer);
    m_buffer.Optimize();
}

void DynamicHuffmanCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    Compress(ioBuffer);

    WriteKeyUsingTree(keyEnd);

    if (ioBuffer.empty())
    {
        m_buffer.Pop(ioBuffer,true);
    }
    else
    {
        std::vector<unsigned char> temp;
        m_buffer.Pop(temp,true);
        ioBuffer.insert(ioBuffer.end(), temp.begin(), temp.end());
    }
}

DynamicHuffmanDeCompressor::DynamicHuffmanDeCompressor()
    : DynamicHuffmanCommon()
    , m_currentNode(m_tree)
{
}

void DynamicHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
    m_buffer.Reserve(ioBuffer.size()*sizeof(ioBuffer[0])*8);
    m_buffer.Push(ioBuffer, static_cast<unsigned int>(ioBuffer.size() * 8));
    ioBuffer.clear();
    bool run = true;
    while (run)
    {
        unsigned int index;
        while (m_currentNode->type == NodeType::branch)
        {
            run = m_buffer.TryPopBit(index);
            if (run)
            {
                m_currentNode = m_currentNode->node[index];
            }
            else
            {
                break;
            }
        }
        if (run)
        {
            switch (m_currentNode->key->value)
            {
            case keyNew:
                if (m_buffer.TryPop(index, 8u))
                {
                    ioBuffer.emplace_back(static_cast<unsigned char>(index));
                    assert(m_keys[index].count == 0);
                    m_keys[keyNew].count++;
                    UpdateTree(index, true);
                    m_currentNode = m_tree;
                }
                else
                {
                    run = false;
                }
                break;
            case keyEnd:
                // see if the filling bits are 0
                if (m_buffer.Size() < 8)
                {
                    unsigned int i = m_buffer.Pop(static_cast<unsigned int>(m_buffer.Size()));
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
                UpdateTree(m_currentNode->key->value, false);
                m_currentNode = m_tree;
                break;
            }
        }
    }
    m_buffer.Optimize();
}

void DynamicHuffmanDeCompressor::Finish(std::vector<unsigned char>& ioBuffer)
{
    DeCompress(ioBuffer);
    if (!m_buffer.Empty() ||
        m_currentNode->type == NodeType::branch ||
        m_currentNode->key->value != keyEnd)
    {
        throw std::runtime_error("Incomplete data");
    }
}


