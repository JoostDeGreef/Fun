#include <memory>

#include "CommonTestFunctionality.h"

#include "ICompress.h"

class CompressTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    std::vector<unsigned char> GetInputData()
    {
        return
        {
            '1','1','2','2','3','3','4','4','5','5',
            '1','1','2','2','3','3','4','4','5',
            '1','1','2','2','3','3','4','4',
            '1','1','2','2','3','3','4',
            '1','1','2','2','3','3',
            '1','1','2','2','3',
            '1','1','2','2',
            '1','1','2',
            '1','1',
            '1',
        };
    }

};

TEST_F(CompressTest, Construct)
{
    std::vector<unsigned char> input = GetInputData();
    auto compressor = CompressorFactory::Create(CompressorType::PassThrough);
    auto deCompressor = DeCompressorFactory::Create(CompressorType::PassThrough);
    auto compressed = input;
    compressor->Finish(compressed);
    auto deCompressed = compressed;
    deCompressor->Finish(deCompressed);
    ASSERT_EQ(input,deCompressed);
    SUCCEED() << "ratio: " << (double)compressed.size() / (double)input.size();
}

