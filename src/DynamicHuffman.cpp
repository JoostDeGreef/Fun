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
}

void DynamicHuffmanDeCompressor::DeCompress(std::vector<unsigned char>& ioBuffer)
{
    m_buffer.Push(ioBuffer, static_cast<unsigned int>(ioBuffer.size() * 8));
    ioBuffer.clear();
    bool run = true;
    while (run)
    {
        if (m_currentNode->type == NodeType::branch)
        {
            unsigned int index;
            if (m_buffer.BitsAvailable() >= m_keys[keyEnd].bits.Length())
            {
                for (; m_currentNode->type != NodeType::leaf;)
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
                    UpdateTree(c,true);
                    m_currentNode = m_tree;
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
                // stop any further actions.
                run = false;
                break;
            default:
                ioBuffer.emplace_back(static_cast<unsigned char>(m_currentNode->key->value));
                assert(m_keys[m_currentNode->key->value].count != 0);
                UpdateTree(m_currentNode->key->value,false);
                m_currentNode = m_tree;
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
