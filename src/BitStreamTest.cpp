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

TEST_F(BitStreamTest, PushTryPopPopBackOnly)
{
    BitStream bs;
    unsigned int data;
    bs.Push(2, 2);
    EXPECT_EQ(2, bs.BitsAvailable());
    EXPECT_FALSE(bs.TryPop(data, 3));
    EXPECT_TRUE(bs.TryPop(data, 1));
    EXPECT_EQ(0, data);
    EXPECT_EQ(1, bs.BitsAvailable());
    data = bs.Pop(1);
    EXPECT_EQ(1, data);
    EXPECT_EQ(0, bs.BitsAvailable());
}

TEST_F(BitStreamTest, PushTryPopPopBuffered)
{
    BitStream bs;
    unsigned int data;
    bs.Push(2, 2);
    EXPECT_EQ(2, bs.BitsAvailable());
    EXPECT_FALSE(bs.TryPop(data, 3));
    bs.Push(42, 8);
    EXPECT_TRUE(bs.TryPop(data, 1));
    EXPECT_EQ(0, data);
    EXPECT_EQ(9, bs.BitsAvailable());
    data = bs.Pop(1);
    EXPECT_EQ(1, data);
    EXPECT_EQ(8, bs.BitsAvailable());
    data = bs.Pop(8);
    EXPECT_EQ(42, data);
}

TEST_F(BitStreamTest, PushTryPopPopMany)
{
    BitStream bs;
    unsigned int data;
    static const std::vector<std::pair<unsigned int, unsigned int>> input = { {1,1},{34,6},{123,7},{127,8},{5,4} };
    for (const auto& i : input)
    {
        bs.Push(i.first, i.second);
    }
    for (const auto& i : input)
    {
        data = bs.Pop(i.second);
        EXPECT_EQ(i.first,data);
    }
    EXPECT_EQ(0, bs.BitsAvailable());
}

