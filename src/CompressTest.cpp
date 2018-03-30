#include <memory>

#include "CommonTestFunctionality.h"

enum class InputType
{
    Random,
    Decreasing,
    Alternating,
};

class CompressTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    std::vector<unsigned char> GetInputData(InputType type)
    {
        static const std::vector<unsigned char> alternating_values = {1,2,3,4};
        std::vector<unsigned char> res;
        switch(type)
        {
        case InputType::Random:
            break;
        case InputType::Decreasing:
            for(int i=50;i>0;--i)
            {
                for(int j=0;j<i;++j)
                {
                    res.emplace_back(static_cast<unsigned char>(i));
                }
            }
            break;
        case InputType::Alternating:
            for(int i=0;i<10000;++i)
            {
                res.insert(res.end(),alternating_values.begin(),alternating_values.end());
            }
            break;
        default:
            assert(false);
            break;
        }
        return res;
    }

};

inline std::ostream& operator<<(std::ostream& stream, InputType const& it)
{
    switch (it)
    {
    case InputType::Random:      return stream << "Random";
    case InputType::Decreasing:  return stream << "Decreasing";
    default: 
        assert(false);
        return stream << "Unknown(" << static_cast<int>(it) << ")";
    }
}

TEST_F(CompressTest, Ratio)
{
    for (auto compressorType : { CompressorType::PassThrough, CompressorType::RLE, CompressorType::Window, CompressorType::StaticHuffman })
    {
        SUCCEED() << "compressor type = " << compressorType;
        for(auto inputType : { InputType::Random, InputType::Decreasing })
        {
            std::vector<unsigned char> input = GetInputData(inputType);
            auto compressor = CompressorFactory::Create(compressorType);
            auto deCompressor = DeCompressorFactory::Create(compressorType);
            auto compressed = input;
            compressor->Finish(compressed);
            auto deCompressed = compressed;
            deCompressor->Finish(deCompressed);
            EXPECT_EQ(input, deCompressed);
            SUCCEED() << "  ratio for " << inputType << ": " << (double)compressed.size() / (double)input.size();
        }
    }
}


