#include <memory>
#include <random>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>
#include <iostream>

#include "CommonTestFunctionality.h"

enum class InputType
{
    Random,       // random values [0..256)
    RandomRange,  // random values [0..10)
    Sequence,     // repeat sequence (1,2,3,4,5,6)
    Sawtooth,     // sawtooth 1-50 times values (1,2,3,4,5,6)
    Single,       // one value (42)
};

class CompressTest : public Test
{
protected:
#ifdef _DEBUG
    static const size_t m_size = 10000;
//    static const size_t m_size = 100;
#else
//    static const size_t m_size = 100000;
    static const size_t m_size = 10000000;
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
                    std::uniform_int_distribution<unsigned int> dist(0, 256);
                    for (size_t i = 0; i < m_size; ++i)
                    {
                        res.emplace_back(static_cast<unsigned char>(dist(rng)));
                    }
                }
                break;
            case InputType::RandomRange:
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
                    auto alternating_values_iter = alternating_values.begin();
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
                            ++alternating_values_iter;
                            if (alternating_values_iter == alternating_values.end())
                            {
                                alternating_values_iter = alternating_values.begin();
                            }
                        }
                        cur++;
                        res.emplace_back(*alternating_values_iter);
                    }
                    break;
                }
                case InputType::Sequence:
                {
                    auto alternating_values_iter = alternating_values.end();
                    while (res.size()<m_size)
                    {
                        if (alternating_values_iter == alternating_values.end())
                        {
                            alternating_values_iter = alternating_values.begin();
                        }
                        res.emplace_back(*alternating_values_iter);
                        ++alternating_values_iter;
                    }
                    break;
                }
                case InputType::Single:
                {
                    while (res.size()<m_size)
                    {
                        res.emplace_back((unsigned char)42);
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

    std::vector<InputType> GetInputTypes() const
    {
        return
        {
            InputType::Random,
            InputType::RandomRange,
            InputType::Sequence,
            InputType::Sawtooth,
            InputType::Single,
        };
    }
};

inline std::string to_string(InputType const& it)
{
    switch (it)
    {
    case InputType::Random:      return "Random";
    case InputType::RandomRange: return "RandomRange";
    case InputType::Sequence:    return "Sequence";
    case InputType::Sawtooth:    return "Sawtooth";
    case InputType::Single:      return "Single";
    default:
        assert(false);
        return "Unknown(" + std::to_string(static_cast<int>(it)) + ")";
    }
}

inline std::ostream& operator<<(std::ostream& stream, InputType const& it)
{
    return stream << to_string(it);
}

TEST_F(CompressTest, Ratio)
{
    class Data
    {
    public:
        Data(const double& ratio,long long msCompres,long long msDeCompres)
            : m_ratio(ratio)
            , m_msCompres(msCompres)
            , m_msDeCompres(msDeCompres)
        {}

        double m_ratio;
        long long m_msCompres;
        long long m_msDeCompres;
    };
    StopWatch sw;
    std::map<InputType, Data> info;
    for (auto inputType : GetInputTypes())
    {
        std::vector<unsigned char> input = GetInputData(inputType);
        auto compressor = CompressorFactory::Create();
        auto deCompressor = DeCompressorFactory::Create();
        auto compressed = input;
        sw.Reset();
        compressor->Finish(compressed);
        auto t0 = sw.GetMS();
        auto deCompressed = compressed;
        sw.Reset();
        deCompressor->Finish(deCompressed);
        auto t1 = sw.GetMS();
        ASSERT_EQ(input.size(), deCompressed.size()) << "InputType: " << inputType;
        ASSERT_EQ(input, deCompressed) << "InputType: " << inputType;
        info.emplace(inputType,Data((double)compressed.size() / (double)input.size(), t0, t1));
    }
    std::stringstream s;
    for (auto it : info)
    {
        s << "  " << it.first;
    }
    s << "   Time (ms)";
    SUCCEED() << s.str();
    std::cout << s.str() << std::endl;
    long long t0 = 0, t1 = 0;
    std::stringstream().swap(s);
    s << "  ";
    for (auto it : info)
    {
        size_t l = to_string(it.first).size();
        auto r = std::to_string(static_cast<int>(it.second.m_ratio * 100));
        s << std::string(l-r.size()-2, ' ') << r << " %  ";
        t0 += it.second.m_msCompres;
        t1 += it.second.m_msDeCompres;
    }
    s << " " << t0 << "/" << t1;
    SUCCEED() << s.str();
    std::cout << s.str() << std::endl;
}

TEST_F(CompressTest, DISABLED_RatioOri)
{
    for (auto inputType : GetInputTypes())
    {
        std::vector<unsigned char> input = GetInputData(inputType);
        auto compressor = CompressorFactory::Create();
        auto deCompressor = DeCompressorFactory::Create();
        auto compressed = input;
        compressor->Finish(compressed);
        auto deCompressed = compressed;
        deCompressor->Finish(deCompressed);
        EXPECT_EQ(input, deCompressed);
        SUCCEED() << "  ratio for " << inputType << ": " << (double)compressed.size() / (double)input.size();
    }
}

