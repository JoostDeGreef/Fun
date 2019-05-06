#include <memory>
#include <random>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>
#include <iostream>

#include "CommonTestFunctionality.h"

class OperatorTest : public Test
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

TEST_F(OperatorTest, Init)
{
    Operator op0(Operator::Value::Plus);
    Operator op1 = Operator::Value::Plus;
    EXPECT_TRUE(op0 == op1);
}

