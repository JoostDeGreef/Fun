#pragma once

#include <tuple>
#include <array>
#include <unordered_set>
#include <utility>

template <typename TYPE,typename ... TYPES>
class grid
{
public:
    const static size_t column_count = sizeof...(TYPES) + 1;
    using this_type = grid<TYPE, TYPES...>;
    using data_tuple = std::tuple<TYPE, TYPES...>;
    template<size_t I>
    struct column
    {
        using type = typename std::tuple_element<I, data_tuple>::type;
    };
private:
    template<size_t I>
    struct column_set
    {
        class grid_hash
        {
        public:
            size_t operator()(const typename column<I>::type &node) const
            {
                return std::hash<column<I>::type>()(node);
            }
            size_t operator()(const data_tuple * const &node) const
            {
                return std::hash<column<I>::type>()(std::get<I>(*node));
            }
        };
        class grid_equal
        {
        public:
            bool operator()(const data_tuple * const &a, const data_tuple * const &b) const
            {
                return std::get<I>(*a) == std::get<I>(*b);
            }
        };
        using type = std::unordered_set<data_tuple*, grid_hash, grid_equal>;
    };
    template<size_t N, typename... D>
    struct data_seq : data_seq<N - 1, typename column_set<N>::type, D...>
    {
    };
    template<typename... D>
    struct data_seq<0, D...>
    {
        using type = std::tuple<typename column_set<0>::type, D...>;
    };
    using data_type = typename data_seq<column_count - 1>::type;
    template<size_t I = column_count - 1>
    class applicator
    {
    public:
        static void emplace(data_type& data, data_tuple* p)
        {
            applicator<I - 1>::emplace(data,p);
            std::get<I>(data).emplace(p);
        }
    };
    template<>
    class applicator<0>
    {
    public:
        static void emplace(data_type& data, data_tuple* p)
        {
            std::get<0>(data).emplace(p);
        }
    };
    data_type m_data;
public:
    ~grid()
    {
        for (auto value : std::get<0>(m_data))
        {
            delete value;
        }
    }

    grid()
        : m_data()
    {}

    grid(const this_type& other)
    {}

    grid(const std::initializer_list<data_tuple>& values)
        : grid()
    {
        for (const auto& value : values)
        {
            auto p = new data_tuple(value);
            applicator<>::emplace(m_data,p);
        }
    }

    template< std::size_t I >
    const auto find(const typename column<I>::type & key) const
    {
        auto s = std::get<I>(m_data);
        auto iter = s.find(key);
        return iter;
    }

};

