#pragma once

#include <tuple>
#include <array>
#include <unordered_set>
#include <utility>
#include <cmath>
#include <algorithm>
#include <ostream>
#include <sstream>

template <typename TYPE,typename ... TYPES>
class grid
{
public:
    const static size_t column_count = sizeof...(TYPES) + 1;
    using this_type = grid<TYPE, TYPES...>;
    using row_type = std::tuple<TYPE, TYPES...>;
    using hash_type = decltype(std::hash<void*>()(nullptr));
    class node_type
    {
    public:
        node_type* m_next;
        node_type* m_prev;
        hash_type m_hash;
        const row_type* m_row;
    };
    using index_type = std::vector<node_type*>;

protected:
    //template<size_t I>
    //static void delete_node(const node_type* node)
    //{
    //    switch (I)
    //    {
    //    case 0:
    //        delete node->m_row;
    //    default:
    //        delete node;
    //    }
    //}
    // msvc: internal compiler error
    template<size_t I>
    static void delete_node(const node_type* node)
    {
        delete node;
    }
    template<>
    static void delete_node<0>(const node_type* node)
    {
        delete node->m_row;
        delete node;
    }

    static size_t get_next_prime(size_t prime)
    {
        static std::vector<size_t> primes = { 2,3,5,7,11 };
        if (primes.back() <= prime)
        {
            for (size_t i = primes.back() + 1; primes.back() <= prime; ++i)
            {
                bool isPrime = true;
                for (const auto p : primes)
                {
                    isPrime = i % p != 0;
                    if (p*p >= i || !isPrime)
                    {
                        break;
                    }
                }
                if (isPrime)
                {
                    primes.push_back(i);
                }
            }
            return primes.back();
        }
        else
        {
            return *std::upper_bound(primes.cbegin(), primes.cend(), prime);
        }
    }
public:

    class iterator : public std::iterator<std::input_iterator_tag,
        node_type,
        ptrdiff_t,
        node_type*,
        node_type&>
    {
    public:
        iterator()
            : m_node()
        {}
        iterator(const iterator & other)
            : m_node(other.m_node)
        {}
        iterator(node_type* node)
            : m_node(node)
        {}
        static iterator begin(node_type* node)
        {
            return iterator(node);
        }
        static iterator end(node_type* node = nullptr)
        {
            return iterator(node);
        }
        operator bool() const
        {
            return m_node != nullptr;
        }
        iterator& operator ++ ()
        {
            m_node = m_node->m_next;
            return *this;
        }
        iterator operator ++ (int)
        {
            iterator res(*this);
            m_node = m_node->m_next;
            return res;
        }
        iterator& operator -- ()
        {
            m_node = m_node->m_prev;
            return *this;
        }
        iterator operator -- (int)
        {
            iterator res(*this);
            m_node = m_node->m_prev;
            return res;
        }
        bool operator != (const iterator& other) const
        {
            return m_node != other.m_node;
        }
        const row_type& operator * () const
        {
            return *get_row();
        }
        const hash_type& get_hash() const
        {
            return m_node->m_hash;
        }
        const node_type* get_node() const
        {
            return m_node;
        }
        const row_type* get_row() const
        {
            return m_node->m_row;
        }
        template<size_t I>
        const auto& get_field() const
        {
            return std::get<I>(*m_node->m_row);
        }
    private:
        node_type* m_node;
    };

    template<size_t I>
    class column_type
    {
    public:
        using key_type = typename std::tuple_element<I, row_type>::type;

        column_type()
            : m_data(nullptr)
            , m_index()
            , m_count(0)
        {
            rehash(4);
        }
        ~column_type()
        {
            clear();
        }

        size_t count() const { return m_count; }
        size_t buckets() const { return m_index.size(); }

        auto begin() const { return iterator::begin(m_data); }
        auto end()   const { return iterator::end(); }

        const auto find(const key_type & key) const { return find_core(key); }
        const auto find(const row_type * row) const { return find_core(row); }
        const auto find_all(const key_type & key) const 
        { 
            auto a = find_core(key);
            auto b = a;
            if (b != end())
            {
                ++b;
                while (b != end() && is_equal(a,b) )
                {
                    ++b;
                }
            }
            return std::make_tuple(iterator(a),iterator(b));
        }
    private:
        static bool is_equal(const key_type & key, const row_type* row) { return std::get<I>(*row) == key; }
        static bool is_equal(const row_type* a, const row_type* b) { return a == b; }
        static bool is_equal(const node_type* a, const node_type* b) { return a->m_hash == b->m_hash && std::get<I>(*a->m_row) == std::get<I>(*b->m_row); }
        static bool is_equal(const key_type & key, const node_type* node) { return std::get<I>(*node->m_row) == key; }
        static bool is_equal(const iterator & a, const iterator & b) { return is_equal(a.get_node(), b.get_node()); }
        template<typename T>
        const auto find_core(const T t) const
        {
            auto h = hash(t);
            auto index = h % buckets();
            node_type* b = m_index[index];
            while (nullptr != b && h == b->m_hash && !is_equal(t, b))
            {
                b = b->m_next;
            }
            if (nullptr == b || h != b->m_hash || !is_equal(t, b))
            {
                b = nullptr;
            }
            return iterator::begin(b);
        }
    public:
        void test_consistency() const
        {
            auto next = m_data;
            decltype(next) prev = nullptr;
            size_t counted = 0;
            while (next)
            {
                assert(next->m_prev == prev);
                assert(counted < m_count);
                counted++;
                prev = next;
                next = next->m_next;
            }
            assert(counted == m_count);
        }
        void insert(const row_type * row)
        {
            test_consistency();
            ++m_count;
            rehash();
            auto h = hash(row);
            node_type* node = new node_type;
            node->m_row = row;
            node->m_hash = h;
            insert_core(node);
            test_consistency();
        }
    private:
        void insert_core(node_type* node)
        {
            node->m_prev = nullptr;
            node->m_next = nullptr;
            auto& row = node->m_row;
            auto& h = node->m_hash;
            auto index = h % buckets();
            node_type* prev_node = nullptr;
            if (nullptr == m_index[index])
            {
                m_index[index] = node;
                node_type* next_node = nullptr;
                for (auto i = index + 1; i < buckets(); ++i)
                {
                    if (m_index[i] != nullptr)
                    {
                        next_node = m_index[i];
                        break;
                    }
                }
                if (next_node != nullptr)
                {
                    prev_node = next_node->m_prev;
                    next_node->m_prev = node;
                }
                else
                {
                    for (auto i = index; i > 0; --i)
                    {
                        if (m_index[i-1] != nullptr)
                        {
                            prev_node = m_index[i-1];
                            while (prev_node->m_next)
                            {
                                prev_node = prev_node->m_next;
                            }
                            prev_node->m_next = node;
                            break;
                        }
                    }
                }
                node->m_next = next_node;
                node->m_prev = prev_node;
            }
            else
            {
                node_type* next_node = m_index[index];
                prev_node = next_node->m_prev;
                while (next_node && next_node->m_hash < h && index == next_node->m_hash % buckets())
                {
                    prev_node = next_node;
                    next_node = next_node->m_next;
                }
                while (next_node && next_node->m_hash == h && std::get<I>(*next_node->m_row) < std::get<I>(*row))
                {
                    prev_node = next_node;
                    next_node = next_node->m_next;
                }
                if (next_node == nullptr)
                {
                    if (prev_node == nullptr)
                    {
                        assert(false);
                        // should be handled in previous section
                    }
                    else
                    {
                        node->m_next = nullptr;
                        node->m_prev = prev_node;
                    }
                }
                else
                {
                    if (prev_node == nullptr)
                    {
                        node->m_next = next_node;
                        node->m_prev = next_node->m_prev;
                        if (node->m_prev != nullptr)
                        {
                            node->m_prev->m_next = node;
                        }
                        next_node->m_prev = node;
                        m_index[index] = node;
                    }
                    else
                    {
                        node->m_next = next_node;
                        node->m_prev = prev_node;
                        next_node->m_prev = node;
                        if (m_index[index] == next_node)
                        {
                            m_index[index] = node;
                        }
                    }
                }
            }
            if (prev_node == nullptr)
            {
                m_data = node;
            }
            else
            {
                prev_node->m_next = node;
            }
            assert(m_data != nullptr);
            assert(m_data->m_prev == nullptr);
        }
    public:
        void erase(const row_type * row)
        {
            --m_count;
            auto h = hash(row);
            auto index = h % buckets();
            auto node = m_index[index];
            while (node && node->m_row != row)
            {
                node = node->m_next;
            }
            assert(node == nullptr || node->m_hash % buckets() == index);
            if (node)
            {
                auto prev = node->m_prev;
                auto next = node->m_next;
                if (m_index[index] == node)
                {
                    if (next && index == next->m_hash % buckets())
                    {
                        m_index[index] = next;
                    }
                    else
                    {
                        m_index[index] = nullptr;
                    }
                }
                if (prev)
                {
                    prev->m_next = next;
                }
                else
                {
                    m_data = next;
                }
                if (next)
                {
                    next->m_prev = prev;
                }
                delete_node<I>(node);
            }
        }
        void rehash(size_t buckets = 0)
        {
            buckets = get_next_prime(std::max(buckets,(size_t)(count()*(1.0 / 0.8))));
            if (buckets > this->buckets() || buckets * 3 + 4 < this->buckets() * 2)
            {
                m_index.resize(buckets);
                std::fill(m_index.begin(), m_index.end(), nullptr);
                node_type* temp = nullptr;
                std::swap(temp,m_data);
                while (temp)
                {
                    auto t = temp;
                    temp = temp->m_next;
                    insert_core(t);
                }
            }
        }
        void clear()
        {
            m_count = 0;
            std::fill(m_index.begin(), m_index.end(), nullptr);
            auto p = m_data;
            m_data = nullptr;
            while (p)
            {
                auto t = p;
                p = p->m_next;
                delete_node<I>(t);
            }
        }
    private:
        size_t m_count;
        node_type* m_data;
        index_type m_index;

        auto hash(const key_type & k) const
        {
            return std::hash<key_type>()(k);
        }
        auto hash(const row_type * const r) const
        {
            return this->hash(std::get<I>(*r));
        }
    };

private:
    template<size_t N, typename... D>
    struct column_seq : column_seq<N - 1, typename column_type<N>, D...>
    {
    };
    template<typename... D>
    struct column_seq<0, D...>
    {
        using type = std::tuple<typename column_type<0>, D...>;
    };
    using columns_type = typename column_seq<column_count - 1>::type;
    columns_type m_columns;

    template<size_t I = column_count - 1>
    class applicator
    {
    public:
        static void insert(columns_type& data, row_type* p)
        {
            applicator<I - 1>::insert(data,p);
            std::get<I>(data).insert(p);
        }
        static void clear(columns_type& data)
        {
            applicator<I - 1>::clear(data);
            std::get<I>(data).clear();
        }
        static void erase(columns_type& data, const row_type* p)
        {
            applicator<I - 1>::erase(data,p);
            std::get<I>(data).erase(p);
        }
        static void to_stream(std::ostream& s, const row_type& row)
        {
            applicator<I - 1>::to_stream(s, row);
            s << "," << std::get<I>(row);
        }
    };
    template<>
    class applicator<0>
    {
    public:
        static void insert(columns_type& data, row_type* p)
        {
            std::get<0>(data).insert(p);
        }
        static void clear(columns_type& data)
        {
            std::get<0>(data).clear();
        }
        static void erase(columns_type& data, const row_type* p)
        {
            std::get<0>(data).erase(p);
        }
        static void to_stream(std::ostream& s, const row_type& row)
        {
            s << std::get<0>(row);
        }
    };
public:
    ~grid()
    {
        clear();
    }

    grid()
        : m_columns()
    {}

    grid(const this_type& other)
    {}

    grid(const std::initializer_list<row_type>& values)
        : grid()
    {
        for (const auto& value : values)
        {
            auto p = new row_type(value);
            applicator<>::insert(m_columns,p);
        }
    }

    // remove all entries
    void clear()
    {
        applicator<>::clear(m_columns);
    }

    // return the first match for key in column<I>
    template< size_t I >
    const auto find(const typename column_type<I>::key_type & key) const
    {
        auto& s = std::get<I>(m_columns);
        return s.find(key);
    }

    // return all matches for key in column<I>
    template< size_t I >
    const auto find_all(const typename column_type<I>::key_type & key) const
    {
        auto& s = std::get<I>(m_columns);
        return s.find_all(key);
    }

    // erase an entry
    template< size_t I = 0>
    void erase(const typename this_type::iterator & iter)
    {
        if(iter)
        {
            applicator<>::erase(m_columns, iter.get_row());
        }
    }
    template< size_t I >
    void erase(const typename column_type<I>::key_type & key)
    {
        auto& s = std::get<I>(m_columns);
        auto& iter = s.find(key);
        erase<I>(iter);
    }

    // insert a new row by supplying values
    template <typename ... T>
    void insert(const T&... t)
    {
        auto p = new row_type(t...);
        applicator<>::insert(m_columns,p);
    }

    // get the value for a column
    template< size_t I, size_t J = I >
    auto get(const typename column_type<I>::key_type & key)
    {
        auto iter = find<I>(key);
        return std::get<J>(*iter);
    }

    // get the number of rows
    auto size() const
    {
        return std::get<0>(m_columns).count();
    }

    auto begin() const 
    { 
        auto& c = std::get<0>(m_columns);
        return c.begin();
    }
    auto end() const 
    { 
        auto& c = std::get<0>(m_columns);
        return c.end();
    }

    std::ostream& to_stream(std::ostream& s) const
    {
        s << "{";
        bool first = true;
        for (const auto& row : std::get<0>(m_columns))
        {
            if (first)
            {
                first = false;
            }
            else
            {
                s << ",";
            }
            s << "{";
            applicator<>::to_stream(s, row);
            s << "}";
        }
        s << "}";
        return s;
    }
};

template<typename ... TYPES>
std::ostream& operator << (std::ostream& s, const grid<TYPES...>& g)
{
    return g.to_stream(s);
}

