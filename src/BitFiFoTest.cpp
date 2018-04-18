#include "CommonTestFunctionality.h"

#include "BitFiFO.h"

class BitFiFoTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(BitFiFoTest, PushTryPopPopBackOnly)
{
    BitFiFo bb;
    unsigned int data;
    bb.Push(2, 2);
    EXPECT_EQ(2ul, bb.Size());
    EXPECT_FALSE(bb.TryPop(data, 3));
    EXPECT_TRUE(bb.TryPop(data, 1));
    EXPECT_EQ(0u, data);
    EXPECT_EQ(1ul, bb.Size());
    data = bb.Pop(1);
    EXPECT_EQ(1u, data);
    EXPECT_EQ(0ul, bb.Size());
}

TEST_F(BitFiFoTest, PushTryPopPopBuffered)
{
    BitFiFo bb;
    unsigned int data;
    bb.Push(2, 2);
    EXPECT_EQ(2ul, bb.Size());
    EXPECT_FALSE(bb.TryPop(data, 3));
    bb.Push(42, 8);
    EXPECT_TRUE(bb.TryPop(data, 1));
    EXPECT_EQ(0u, data);
    EXPECT_EQ(9ul, bb.Size());
    data = bb.Pop(1);
    EXPECT_EQ(1u, data);
    EXPECT_EQ(8ul, bb.Size());
    data = bb.Pop(8);
    EXPECT_EQ(42u, data);
}

TEST_F(BitFiFoTest, PushTryPopPopMany)
{
    BitFiFo bb;
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
    EXPECT_EQ(0ul, bb.Size());
}

TEST_F(BitFiFoTest, FlushRetrieve)
{
    BitFiFo bits;
    std::vector<unsigned char> out;
    for(int i=0;i<100;++i)
    {
        for(int j=0;j<i;++j)
        {
            bits.Push(1u,1u);
        }
        size_t count = bits.Size();
        bits.Pop(out,true);
        EXPECT_LE(count,out.size()*8);
        bits.Clear();
        out.clear();
    }
}

