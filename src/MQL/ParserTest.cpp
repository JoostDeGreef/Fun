#include <memory>
#include <random>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>
#include <iostream>

#include "CommonTestFunctionality.h"

class ParserProxy : public Parser
{
public:
    using Parser::ExtractSymbols;
};

class ParserTest : public Test
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

TEST_F(ParserTest, Symbols)
{
    ParserProxy parser;
    auto res = parser.ExtractSymbols("a = '1';");
    ASSERT_EQ(4, res.size());
}

