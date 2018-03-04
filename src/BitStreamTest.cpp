#include "CommonTestFunctionality.h"

#include "BitStream.h"

class BitStreamTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(BitStreamTest, PushTryPopPop)
{
    BitStream bs;
    unsigned int data;
    bs.Push(2, 2);
    EXPECT_EQ(2, bs.BitsAvailable());
    EXPECT_FALSE(bs.TryPop(data, 3));
    EXPECT_FALSE(bs.TryPop(data, 1));
    EXPECT_EQ(0, data);
    EXPECT_EQ(1, bs.BitsAvailable());
    data = bs.Pop(1);
    EXPECT_EQ(1, data);
    EXPECT_EQ(0, bs.BitsAvailable());
}

