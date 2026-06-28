#pragma once

#include <tuple>
#include <array>
#include <unordered_set>
#include <utility>
#include <cmath>
#include <algorithm>

template <typename TYPE,typename ... TYPES>
class grid
{
public:
    const static size_t column_count = sizeof...(TYPES) + 1;
    using this_type = grid<TYPE, TYPES...>;
    using row_type = std::tuple<TYPE, TYPES...>;
    using node_data_type = std::tuple<decltype(std::hash<void*>()(nullptr)), row_type*>;
 
    template<size_t I>
    class column
    {
    public:
        using key_type = typename std::tuple_element<I, row_type>::type;
        using node_type = std::vector<node_data_type>;
        using data_type = std::vector<node_type>;

        class iterator
        {
            using outer_iter = typename data_type::const_iterator;
            using inner_iter = typename node_type::const_iterator;
            iterator()
                : m_outer()
                , m_outer_cend()
                , m_inner()
            {}
        public:
            iterator(const iterator & other)
                : m_outer(other.m_outer)
                , m_outer_cend(other.m_outer_cend)
                , m_inner(other.m_inner)
            {}
            iterator(const outer_iter &outer, const outer_iter &outer_cend, const inner_iter &inner)
                : m_outer(outer)
                , m_outer_cend(outer_cend)
                , m_inner(inner)
            {}
            static iterator begin(const data_type &dt)
            {
                outer_iter outer = dt.cbegin();
                for (; outer != dt.cend(); ++outer)
                {
                    if (!outer->empty())
                    {
                        return iterator(outer, dt.cend(), outer->cbegin());
                    }
                }
                return end(dt);
            }
            static iterator end(const data_type &dt)
            {
                return iterator(dt.cend(), dt.cend(), inner_iter());
            }
            iterator& operator ++ ()
            {
                ++m_inner;
                if (m_inner == m_outer->cend())
                {
                    ++m_outer;
                    for (; m_outer != m_outer_cend; ++m_outer)
                    {
                        if (!m_outer->empty())
                        {
                            m_inner = m_outer->cbegin();
                            break;
                        }
                    }
                }
                return *this;
            }
            bool operator != (const iterator& other) const
            {
                return m_outer != other.m_outer ||
                    (m_outer == other.m_outer && m_outer != m_outer_cend && m_inner != other.m_inner);
            }
            const row_type * const operator * () const
            {
                return std::get<1>(*m_inner);
            }
            const node_data_type& get_hash_value() const
            {
                return *m_inner;
            }
            template<size_t J = I>
            const auto get() const
            {
                return std::get<J>(*std::get<1>(*m_inner));
            }
        private:
            outer_iter m_outer;
            outer_iter m_outer_cend;
            inner_iter m_inner;
        };

        column()
            : m_data()
            , m_count(0)
        {
            rehash(4);
        }

        size_t count() const { return m_count; }
        size_t buckets() const { return m_data.size(); }

        void insert(const row_type* r) 
        { 
            auto h = this->hash(r); 
        }

        auto begin() const { return iterator::begin(m_data); }
        auto end()   const { return iterator::end(m_data); }

        const auto find(const key_type & key) const { return find_core(key); }
        const auto find(const row_type * row) const { return find_core(row); }
        const auto find_all(const key_type & key) const 
        { 
            auto h = hash(key);
            auto index = h % buckets();
            const auto& bucket = m_data[index];
            std::vector<iterator> res;
            for (auto inner = bucket.cbegin(); inner != bucket.cend(); ++inner)
            {
                if(std::get<0>(*inner) == h && is_equal(key, std::get<1>(*inner)))
                {
                    auto outer = m_data.cbegin() + index;
                    res.emplace_back(outer, m_data.cend(), inner);
                }
            }
            return res;
        }
    private:
        static bool is_equal(const key_type & key, const row_type* row) { return std::get<I>(*row) == key; }
        static bool is_equal(const row_type* a, const row_type* b) { return a == b; }
        template<typename T>
        const auto find_core(const T t) const
        {
            auto h = hash(t);
            auto index = h % buckets();
            const auto& bucket = m_data[index];
            auto inner = std::find_if(bucket.cbegin(), bucket.cend(), [&t,&h](const node_data_type& r) { return std::get<0>(r) == h && is_equal(t,std::get<1>(r)); });
            if (inner != bucket.cend())
            {
                auto outer = m_data.cbegin() + index;
                return iterator(outer, m_data.cend(), inner);
            }
            return end();
        }
    public:
        auto emplace(row_type * row)
        {
            ++m_count;
            rehash();
            auto h = hash(row);
            auto index = h % buckets();
            auto& bucket = m_data[index];
            bucket.emplace_back(h,row);
            auto outer = m_data.cbegin() + index;
            auto inner = bucket.cbegin() + bucket.size() - 1;
            return iterator(outer, m_data.cend(), inner);
        }
        void erase(const row_type * row)
        {
            --m_count;
            auto h = hash(row);
            auto index = h % buckets();
            auto& bucket = m_data[index];
            auto inner = std::find_if(bucket.cbegin(), bucket.cend(), [&row, &h](const node_data_type& r) { return std::get<0>(r) == h && is_equal(row, std::get<1>(r)); });
            bucket.erase(inner);
        }
        void rehash(size_t buckets = 0)
        {
            buckets = get_next_prime(std::max(buckets,(size_t)(count()*(1.0 / 0.8))));
            if (buckets > this->buckets() || buckets * 3 < this->buckets() * 2)
            {
                data_type temp;
                temp.resize(buckets);
                for (auto iter = begin(); iter != end(); ++iter)
                {
                    auto hv = iter.get_hash_value();
                    auto h = std::get<0>(hv);
                    auto row = std::get<1>(hv);
                    auto index = h % buckets;
                    temp[index].emplace_back(h,row);
                }
                m_data.swap(temp);
            }
        }
        void clear()
        {
            m_count = 0;
            for (auto& inner : m_data)
            {
                inner.clear();
            }
        }
    private:
        size_t m_count;
        data_type m_data;

        size_t get_next_prime(size_t prime)
        {
            static std::vector<size_t> primes = { 2,3,5,7,11 };
            if (primes.back() <= prime)
            {
                for (size_t i = primes.back() + 1; primes.back() <= prime; ++i)
                {
                    bool isPrime = true;
                    for (size_t j = 0; ((uint64_t)primes[j]) * primes[j] <= ((uint64_t)i)*i && isPrime; ++j)
                    { 
                        isPrime = (i % j == 0);
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
    struct column_seq : column_seq<N - 1, typename column<N>, D...>
    {
    };
    template<typename... D>
    struct column_seq<0, D...>
    {
        using type = std::tuple<typename column<0>, D...>;
    };
    using columns_type = typename column_seq<column_count - 1>::type;
    columns_type m_columns;

    template<size_t I = column_count - 1>
    class applicator
    {
    public:
        static void emplace(columns_type& data, row_type* p)
        {
            applicator<I - 1>::emplace(data,p);
            std::get<I>(data).emplace(p);
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
    };
    template<>
    class applicator<0>
    {
    public:
        static void emplace(columns_type& data, row_type* p)
        {
            std::get<0>(data).emplace(p);
        }
        static void clear(columns_type& data)
        {
            std::get<0>(data).clear();
        }
        static void erase(columns_type& data, const row_type* p)
        {
            std::get<0>(data).erase(p);
        }
    };
public:
    ~grid()
    {
        for (const auto& value : std::get<0>(m_columns))
        {
            delete value;
        }
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
            applicator<>::emplace(m_columns,p);
        }
    }

    // remove all entries
    void clear()
    {
        for (const auto& value : std::get<0>(m_columns))
        {
            delete value;
        }
        applicator<>::clear(m_columns);
    }

    // return the first match for key in column<I>
    template< size_t I >
    const auto find(const typename column<I>::key_type & key) const
    {
        auto& s = std::get<I>(m_columns);
        return s.find(key);
    }

    // return all matches for key in column<I>
    template< size_t I >
    const auto find_all(const typename column<I>::key_type & key) const
    {
        auto& s = std::get<I>(m_columns);
        return s.find_all(key);
    }

    // erase an entry
    template< size_t I >
    void erase(const typename column<I>::key_type & key)
    {
        auto& s = std::get<I>(m_columns);
        auto& iter = s.find(key);
        auto p = *iter;
        applicator<>::erase(m_columns, *iter);
        delete p;
    }

    // get the value for a column
    template< size_t I, size_t J = I >
    auto get(const typename column<I>::key_type & key)
    {
        auto iter = find<I>(key);
        return iter.get<J>();
    }

    // get the number of rows
    auto size() const
    {
        return std::get<0>(m_columns).count();
    }
};

