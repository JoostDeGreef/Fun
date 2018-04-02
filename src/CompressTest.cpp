#include <memory>
#include <random>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>

#include "CommonTestFunctionality.h"

enum class InputType
{
    Random,       // random values [0..10]
    Sequence,     // repeat sequence (1,2,3,4,5,6)
    Sawtooth,     // sawtooth 1-50 times values (1,2,3,4,5,6)
};

class CompressTest : public Test
{
protected:
#ifdef _DEBUG
    static const size_t m_size = 10000;
#else
    static const size_t m_size = 1000000;
#endif

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    std::vector<unsigned char> GetInputData(InputType type)
    {
        static const std::vector<unsigned char> alternating_values = {1,2,3,4,5,6};
        static std::map<InputType, std::vector<unsigned char >> inputs;
        auto iter = inputs.find(type);
        if (iter == inputs.end())
        {
            std::vector<unsigned char> res;
            switch (type)
            {
            case InputType::Random:
                {
                    std::mt19937 rng;
                    rng.seed(0); // make test repeatable
                    std::uniform_int_distribution<unsigned int> dist(0, 10);
                    for (size_t i = 0; i < m_size; ++i)
                    {
                        res.emplace_back(static_cast<unsigned char>(dist(rng)));
                    }
                }
                break;
            case InputType::Sawtooth:
                {
                    auto iter = alternating_values.begin();
                    size_t max = 0;
                    size_t cur = 0;
                    while (res.size()<m_size)
                    {
                        if (cur >= max)
                        {
                            max++;
                            if (max >= 50)
                            {
                                max = 1;
                            }
                            cur = 0;
                            ++iter;
                            if (iter == alternating_values.end())
                            {
                                iter = alternating_values.begin();
                            }
                        }
                        cur++;
                        res.emplace_back(*iter);
                    }
                    break;
                }
                case InputType::Sequence:
                {
                    auto iter = alternating_values.end();
                    while (res.size()<m_size)
                    {
                        if (iter == alternating_values.end())
                        {
                            iter = alternating_values.begin();
                        }
                        res.emplace_back(*iter);
                        ++iter;
                    }
                    break;
                }
            default:
                assert(false);
                break;
            }
            iter = inputs.emplace(type,res).first;
        }
        return iter->second;
    }

    std::vector<CompressorType> GetCompressorTypes() const
    {
        return
        { 
            CompressorType::PassThrough,
            CompressorType::RLE,
            CompressorType::Window,
            CompressorType::StaticHuffman,
            CompressorType::StaticBlockHuffman,
            CompressorType::RLE_StaticHuffman,
            CompressorType::RLE_StaticBlockHuffman
        };
    }

    std::vector<InputType> GetInputTypes() const
    {
        return
        {
            InputType::Random,
            InputType::Sequence,
            InputType::Sawtooth
        };
    }
};

inline std::string to_string(InputType const& it)
{
    switch (it)
    {
    case InputType::Random:      return "Random";
    case InputType::Sequence:    return "Sequence";
    case InputType::Sawtooth:    return "Sawtooth";
    default:
        assert(false);
        return "Unknown(" + std::to_string(static_cast<int>(it)) + ")";
    }
}

inline std::ostream& operator<<(std::ostream& stream, InputType const& it)
{
    return stream << to_string(it);
}

inline std::string to_string(CompressorType const& ct)
{
    switch (ct)
    {
    case CompressorType::PassThrough:            return "PassThrough";
    case CompressorType::RLE:                    return "RLE";
    case CompressorType::Window:                 return "Window";
    case CompressorType::StaticHuffman:          return "StaticHuffman";
    case CompressorType::StaticBlockHuffman:     return "StaticBlockHuffman";
    case CompressorType::RLE_StaticHuffman:      return "RLE_StaticHuffman";
    case CompressorType::RLE_StaticBlockHuffman: return "RLE_StaticBlockHuffman";
    default:
        assert(false);
        return  "Unknown(" + std::to_string(static_cast<int>(ct)) + ")";
    }
}

inline std::ostream& operator<<(std::ostream& stream, CompressorType const& ct)
{
    return stream << to_string(ct);
}

TEST_F(CompressTest, Ratio)
{
    class Data
    {
    public:
        Data(const double& ratio)
            : m_ratio(ratio)
        {}

        double m_ratio;
    };
    size_t compressorTypeLength = 0;
    std::map<CompressorType, std::map<InputType, Data>> info;
    for (auto compressorType : GetCompressorTypes())
    {
        compressorTypeLength = std::max(compressorTypeLength,to_string(compressorType).size());
        auto iter = info.emplace(compressorType, std::map<InputType, Data>()).first;
        for (auto inputType : GetInputTypes())
        {
            std::vector<unsigned char> input = GetInputData(inputType);
            auto compressor = CompressorFactory::Create(compressorType);
            auto deCompressor = DeCompressorFactory::Create(compressorType);
            auto compressed = input;
            compressor->Finish(compressed);
            auto deCompressed = compressed;
            deCompressor->Finish(deCompressed);
            EXPECT_EQ(input, deCompressed);
            iter->second.emplace(inputType,Data((double)compressed.size() / (double)input.size()));
        }
    }
    std::stringstream s;
    s << std::string(compressorTypeLength, ' ');
    for (auto it : info.begin()->second)
    {
        s << "  " << it.first;
    }
    SUCCEED() << s.str();
    for(auto ct: info)
    {
        std::stringstream().swap(s);
        std::string c = to_string(ct.first);
        s << std::string(compressorTypeLength - c.size(),' ') << c << ": ";
        for (auto it : ct.second)
        {
            size_t l = to_string(it.first).size();
            auto r = std::to_string(static_cast<int>(it.second.m_ratio * 100));
            s << std::string(l-r.size()-2, ' ') << r << " %  ";
        }
        SUCCEED() << s.str();
    }
}

TEST_F(CompressTest, DISABLED_RatioOri)
{
    for (auto compressorType : { CompressorType::PassThrough,
        CompressorType::RLE,
        CompressorType::Window,
        CompressorType::StaticHuffman,
        CompressorType::StaticBlockHuffman,
        CompressorType::RLE_StaticHuffman,
        CompressorType::RLE_StaticBlockHuffman })
    {
        SUCCEED() << "compressor type = " << compressorType;
        for (auto inputType : { InputType::Random,
            InputType::Sequence,
            InputType::Sawtooth })
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

