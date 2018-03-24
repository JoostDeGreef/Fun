#include <memory>

#include "CommonTestFunctionality.h"

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
            '1','1','1','1','1','1','1','1','1','1',
            '2','2','2','2','2','2','2','2','2',
            '3','3','3','3','3','3','3','3',
            '4','4','4','4','4','4','4',
            '5','5','5','5','5','5',
            '6','6','6','6','6',
            '7','7','7','7',
            '8','8','8',
            '9','9',
            '0',
        };
    }

};

TEST_F(CompressTest, Finish)
{
    for (auto type : { CompressorType::PassThrough, CompressorType::RLE, CompressorType::Window, CompressorType::StaticHuffman })
    {
        std::vector<unsigned char> input = GetInputData();
        auto compressor = CompressorFactory::Create(type);
        auto deCompressor = DeCompressorFactory::Create(type);
        auto compressed = input;
        compressor->Finish(compressed);
        auto deCompressed = compressed;
        deCompressor->Finish(deCompressed);
        ASSERT_EQ(input, deCompressed);
        SUCCEED() << "ratio for type " << type << ": " << (double)compressed.size() / (double)input.size();
    }
}

