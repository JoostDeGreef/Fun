#include <memory>
#include <random>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>
#include <iostream>
#include <sstream>

#include "CommonTestFunctionality.h"

class GridTest : public Test
{
protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

};

//inline std::string to_string(InputType const& it)
//{
//    switch (it)
//    {
//    case InputType::Random:      return "Random";
//    case InputType::RandomRange: return "RandomRange";
//    case InputType::Sequence:    return "Sequence";
//    case InputType::Sawtooth:    return "Sawtooth";
//    case InputType::Single:      return "Single";
//    default:
//        assert(false);
//        return "Unknown(" + std::to_string(static_cast<int>(it)) + ")";
//    }
//}
//
//inline std::ostream& operator<<(std::ostream& stream, InputType const& it)
//{
//    return stream << to_string(it);
//}

TEST_F(GridTest, Find)
{
    grid<int, int> g0;
    grid<int, long> g1 = { {1,11},{2,22},{3,33} };
    SUCCEED() << g1;
    const auto& i0 = *g1.find<0>(2);
    const auto& i1 = *g1.find<1>(22);
    ASSERT_EQ(i0, i1);
}

TEST_F(GridTest, FindAll)
{
    grid<int, long> g1 = { {1,11},{2,22},{3,33},{2,222} };
    const auto i0 = g1.find_all<0>(2);
    const auto dist = std::distance(std::get<0>(i0), std::get<1>(i0));
    ASSERT_EQ(2, dist);
}

TEST_F(GridTest, Get)
{
    grid<int, long> g1 = { {1,11},{2,22},{3,33},{2,222} };
    const auto i0 = g1.get<0,1>(3);
    ASSERT_EQ(33, i0);
}

TEST_F(GridTest, Size)
{
    grid<int, long> g1 = { {1,11},{2,22},{3,33},{2,222},{5,55},{23,23},{23,64} };
    const auto i0 = g1.size();
    ASSERT_EQ(7, i0);
}

TEST_F(GridTest, Clear)
{
    grid<int, long> g1 = { {1,11},{2,22},{3,33},{2,222} };
    g1.clear();
    const auto s0 = g1.size();
    ASSERT_EQ(0, s0);
}

TEST_F(GridTest, Erase)
{
    grid<int, long> g1 = { {1,11},{2,22},{3,33},{2,222} };
    g1.erase<0>(3);
    auto s0 = g1.size();
    ASSERT_EQ(3, s0);
    const auto i0 = g1.find<0>(2);
    g1.erase<0>(i0);
    s0 = g1.size();
    ASSERT_EQ(2, s0);
}

TEST_F(GridTest, Insert)
{
    grid<int, long> g1 = { {1,11},{2,22},{3,33},{2,222} };
    g1.insert(4,44);
    const auto s0 = g1.size();
    ASSERT_EQ(5, s0);
}

TEST_F(GridTest, ToString)
{
    grid<int, long> g1 = { {1,11},{2,22},{3,33},{2,222} };
    std::stringstream ss;
    ss << g1;
    std::string s = ss.str();
    ASSERT_EQ("{{2,222},{2,22},{1,11},{3,33}}", s); // sorted by hash!
}
