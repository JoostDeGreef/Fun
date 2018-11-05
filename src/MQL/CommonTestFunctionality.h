#pragma once

#include <chrono>

#include "gtest/gtest.h"
using namespace testing;

#include "Parser.h"
#include "Symbol.h"
#include "Operator.h"

inline std::string to_string(Symbol const& s)
{
    switch (s.GetType())
    {
    case Symbol::Type::HardText: return "Symbol::HardText(" + s.GetHardText() + ")";
    case Symbol::Type::SoftText: return "Symbol::SoftText(" + s.GetSoftText() + ")";
    case Symbol::Type::Keyword: return "Symbol::Keyword(" + s.GetKeyword() + ")";
    case Symbol::Type::Operator: return "Symbol::Operator(" + s.GetOperator().GetText() + ")";
    case Symbol::Type::Uninitialized: return "Symbol::Uninitialized()";
    default:
        assert(false);
        return "Unknown(" + std::to_string(static_cast<int>(s.GetType())) + ")";
    }
}

inline std::ostream& operator<<(std::ostream& stream, Symbol const& s)
{
    return stream << to_string(s);
}

class StopWatch
{
public:
    StopWatch()
    {
        Reset();
    }

    void Reset()
    {
        m_last = m_clock.now();
    }

    long long GetMS()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now() - m_last).count();
    }
private:
    std::chrono::high_resolution_clock m_clock;
    std::chrono::high_resolution_clock::time_point m_last;
};
