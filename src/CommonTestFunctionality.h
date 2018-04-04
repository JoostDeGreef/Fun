#pragma once

#include <chrono>

#include "gtest/gtest.h"
using namespace testing;

#include "ICompress.h"

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
