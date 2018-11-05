#include <memory>
#include <random>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>
#include <iostream>

#include "CommonTestFunctionality.h"

class SymbolTest : public Test
{
protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

};

TEST_F(SymbolTest, Init)
{
    Symbol s1;
    Symbol s2 = Symbol::Keyword("test");
    s1 = s2;
    ASSERT_EQ(s1,s2);
}

