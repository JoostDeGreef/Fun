#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <inttypes.h>
#include <type_traits>

class BitFiFo
{
private:
    typedef uint64_t data_type;
    static const size_t data_bits = sizeof(data_type) * 8;

public:
    BitFiFo()
    {
        m_data.emplace_back();
        Clear();
    }

    BitFiFo(BitFiFo&& other)
    {
        Swap(other);
    }

    BitFiFo(const BitFiFo& other)
    {
        Copy(other);
    }

    void Swap(BitFiFo& other)
    {
        m_data.swap(other.m_data);
        std::swap(m_begin, other.m_begin);
        std::swap(m_end, other.m_end);
    }

    void Copy(const BitFiFo& other)
    {
        m_data = other.m_data;
        m_begin = other.m_begin;
        m_end = other.m_end;
    }

    void Clear()
    {
        for (auto& data : m_data)
        {
            data = 0;
        }
        m_begin = 0;
        m_end = 0;
    }

    size_t Size() const
    {
        return m_end - m_begin;
    }
    bool Empty() const
    {
        return m_end == m_begin;
    }

    template<typename INTEGER>
    void Push(INTEGER data, const size_t bits)
    {
        assert(bits <= sizeof(data) * 8);
        assert(((data_type)data)>>bits==0 || bits==data_bits);
        if (m_end+bits >= data_bits*m_data.size())
        {
            m_data.emplace_back(0);
        }
        size_t offset = m_end % data_bits;
        m_data[m_end / data_bits] |= ((data_type)data) << offset;
        if (data_bits - offset < bits)
        {
            m_data[1 + m_end / data_bits] |= ((data_type)data) >> (data_bits - offset);
        }
        m_end += bits;
    }

    template<typename INTEGER>
    void Push(const std::vector<INTEGER>& data, const size_t bits)
    {
#ifdef _DEBUG
        size_t size = Size();
#endif // _DEBUG
        assert(data.size() == (bits + sizeof(INTEGER)*8 - 1) / (sizeof(INTEGER)*8));
        auto iter = data.cbegin();
        auto end = data.cbegin() + bits / (sizeof(INTEGER) * 8);
        for (; iter!=end ; ++iter)
        {
            Push(*iter, sizeof(INTEGER) * 8);
        }
        if (bits % (sizeof(INTEGER) * 8) != 0)
        {
            Push(*iter, bits % (sizeof(INTEGER) * 8));
        }
#ifdef _DEBUG
        assert(size + bits == Size());
#endif // _DEBUG
    }

    void Push(const BitFiFo& other)
    {
#ifdef _DEBUG
        size_t size = Size();
#endif // _DEBUG
        auto iter = other.m_data.begin() + other.m_begin / data_bits;
        auto end = other.m_data.begin() + other.m_end / data_bits;
        data_type val;
        const size_t prebits = data_bits - (other.m_begin % data_bits);
        if (prebits < data_bits)
        {
            val = *iter >> (data_bits - prebits);
            if (prebits < other.Size())
            {
                Push(val, prebits);
                ++iter;
            }
            else
            {
                Push(val, other.Size());
                return;
            }
        }
        for (;iter != end;++iter)
        {
            val = *iter;
            Push(val, data_bits);
        }
        const size_t postbits = other.m_end % data_bits;
        if (postbits > 0)
        {
            val = *iter;
            Push(val, postbits);
        }
#ifdef _DEBUG
        assert(size + other.Size() == Size());
#endif // _DEBUG
    }

    unsigned int Pop(const size_t bits)
    {
        unsigned int res = Peek(bits);
        m_begin += bits;
        assert(m_begin <= m_end);
        return res;
    }
    bool TryPop(unsigned int& data, unsigned int bits)
    {
        bool res = TryPeek(data,bits);
        if (res)
        {
            m_begin += bits;
            assert(m_begin <= m_end);
        }
        return res;
    }

    unsigned int Peek(const size_t bits)
    {
        unsigned int data = 0;
        bool res = TryPeek(data, bits);
        assert(res);
        return data;
    }
    bool TryPeek(unsigned int& data, size_t bits);

private:
    template<typename INTEGER>
    void Flush() // fill the buffer with 0 so the size if a multitude of sizeof(INTEGER)*8
    {
        Flush(sizeof(INTEGER) * 8);
    }
    void Flush(const size_t granularity) // fill the buffer with 0 so the size % granularity = 0
    {
        if (Size() % granularity > 0)
        {
            Push(0, granularity - (Size() % granularity));
        }
    }

public:
    template<typename INTEGER>
    void Pop(std::vector<INTEGER>& outBuffer,const bool flush)
    {
        if (flush)
        {
            Flush<INTEGER>();
        }
        Pop(outBuffer);
    }
    template<typename INTEGER>
    void Pop(std::vector<INTEGER>& outBuffer)
    {
        // todo: improve this
        while (Size() >= sizeof(INTEGER)*8)
        {
            outBuffer.emplace_back((INTEGER)Peek(sizeof(INTEGER)*8));
            m_begin += sizeof(INTEGER)*8;
            assert(m_begin <= m_end);
        }
    }

    void Optimize()
    {
        if (m_begin > data_bits * 4096)
        {
            if (m_begin == m_end)
            {
                Clear();
            }
            else
            {
                const size_t remove = m_begin / data_bits;
                m_data.erase(m_data.begin(), m_data.begin() + remove);
                m_begin -= data_bits * remove;
                m_end -= data_bits * remove;
            }
        }
    }

    std::vector<data_type> m_data;
    size_t m_begin;
    size_t m_end;
};
