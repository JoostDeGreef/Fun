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
 
    template<size_t I>
    class column
    {
    public:
        using key_type = typename std::tuple_element<I, row_type>::type;
        using node_type = std::vector<row_type*>;
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
            row_type * const operator * () const
            {
                return *m_inner;
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
            auto h = this->hash(r)
        }

        auto begin() const { return iterator::begin(m_data); }
        auto begin() { return iterator::begin(m_data); }
        auto end()   const { return iterator::end(m_data); }
        auto end() { return iterator::end(m_data); }

        const auto find(const key_type & key) const
        {
            auto index = hash(key) % buckets();
            const auto& bucket = m_data[index];
            auto inner = std::find_if(bucket.cbegin(), bucket.cend(), [&key](const row_type* row) { return std::get<I>(*row) == key; });
            if (inner != bucket.cend())
            {
                auto outer = m_data.cbegin() + index;
                return iterator(outer, m_data.cend(), inner);
            }
            return end();
        }
        const auto find(const row_type * row) const
        {
            auto index = hash(row) % buckets();
            const auto& bucket = m_data[index];
            auto inner = std::find_if(bucket.cbegin(), bucket.cend(), [&row](const row_type* r) { return row == r; });
            if (inner != bucket.cend())
            {
                auto outer = m_data.cbegin() + index;
                return iterator(outer, m_data.cend(), inner);
            }
            return end();
        }
        auto emplace(row_type * row)
        {
            ++m_count;
            rehash();
            auto index = hash(row) % buckets();
            auto& bucket = m_data[index];
            bucket.emplace_back(row);
            auto outer = m_data.cbegin() + index;
            auto inner = bucket.cbegin() + bucket.size() - 1;
            return iterator(outer, m_data.cend(), inner);
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
                    auto row = *iter;
                    auto index = hash(row) % buckets;
                    temp[index].emplace_back(row);
                }
                m_data.swap(temp);
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
        bool equal(const row_type * const &a, const row_type * const &b) const
        {
            return std::get<I>(*a) == std::get<I>(*b);
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
    };
    template<>
    class applicator<0>
    {
    public:
        static void emplace(columns_type& data, row_type* p)
        {
            std::get<0>(data).emplace(p);
        }
    };
public:
    ~grid()
    {
        for (auto value : std::get<0>(m_columns))
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

    template< size_t I >
    const auto find(const typename column<I>::key_type & key) const
    {
        auto& s = std::get<I>(m_columns);
        auto iter = s.find(key);
        return iter;
    }

};

