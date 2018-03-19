#include "CommonTestFunctionality.h"

#include "BitBuffer.h"

class BitBufferTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(BitBufferTest, PushTryPopPopBackOnly)
{
    BitBuffer bb;
    unsigned int data;
    bb.Push(2, 2);
    EXPECT_EQ(2, bb.BitsAvailable());
    EXPECT_FALSE(bb.TryPop(data, 3));
    EXPECT_TRUE(bb.TryPop(data, 1));
    EXPECT_EQ(0, data);
    EXPECT_EQ(1, bb.BitsAvailable());
    data = bb.Pop(1);
    EXPECT_EQ(1, data);
    EXPECT_EQ(0, bb.BitsAvailable());
}

TEST_F(BitBufferTest, PushTryPopPopBuffered)
{
    BitBuffer bb;
    unsigned int data;
    bb.Push(2, 2);
    EXPECT_EQ(2, bb.BitsAvailable());
    EXPECT_FALSE(bb.TryPop(data, 3));
    bb.Push(42, 8);
    EXPECT_TRUE(bb.TryPop(data, 1));
    EXPECT_EQ(0, data);
    EXPECT_EQ(9, bb.BitsAvailable());
    data = bb.Pop(1);
    EXPECT_EQ(1, data);
    EXPECT_EQ(8, bb.BitsAvailable());
    data = bb.Pop(8);
    EXPECT_EQ(42, data);
}

TEST_F(BitBufferTest, PushTryPopPopMany)
{
    BitBuffer bb;
    unsigned int data;
    static const std::vector<std::pair<unsigned int, unsigned int>> input = { {1,1},{34,6},{123,7},{127,8},{5,4} };
    for (const auto& i : input)
    {
        bb.Push(i.first, i.second);
    }
    for (const auto& i : input)
    {
        data = bb.Pop(i.second);
        EXPECT_EQ(i.first,data);
    }
    EXPECT_EQ(0, bb.BitsAvailable());
}

