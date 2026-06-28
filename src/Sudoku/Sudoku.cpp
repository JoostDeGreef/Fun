#include "Sudoku.h"

#include <iostream>

namespace impl
{
    constexpr int Row(const int index) { return index / 9; }
    constexpr int Col(const int index) { return index % 9; }
    constexpr int Box(const int index) { return (Row(index) / 3) * 3 + (Col(index) / 3); }
}

Sudoku::Sudoku()
{
    Init();
}

std::string Sudoku::str() const
{
    std::string res;
    res.reserve(9 * 9);
    for(const Field & field : fields)
    { 
        res.push_back('0' + field);
    }
    return res;
}

void Sudoku::Display() const
{
    const char* filler =
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n\0"
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n\0"
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n"
        "------+-------+------\n\0"
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n\0"
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n\0"
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n"
        "------+-------+------\n\0"
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n\0"
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n\0"
        " \0 \0 | \0 \0 \0 | \0 \0 \0\n\0"
        "\0";
    for (const Field& field : fields)
    {
        std::cout << (char)('0' + field);
        while (*filler)
        {
            std::cout << *filler;
            filler++;
        }
        filler++;
    }
}

void Sudoku::Init()
{
    fields.fill(0);
}

namespace impl
{
    template<int INDEX>
    bool Solve(Fields& fields)
    {
        int& value = fields[INDEX];
        if (value)
        {
            return Solve<INDEX + 1>(fields);
        }
        else
        {
            auto RowHasValue = [&](const int value)
                {
                    constexpr auto row = Row(INDEX);
                    return
                        (fields[row * 9 + 0] == value) ||
                        (fields[row * 9 + 1] == value) ||
                        (fields[row * 9 + 2] == value) ||
                        (fields[row * 9 + 3] == value) ||
                        (fields[row * 9 + 4] == value) ||
                        (fields[row * 9 + 5] == value) ||
                        (fields[row * 9 + 6] == value) ||
                        (fields[row * 9 + 7] == value) ||
                        (fields[row * 9 + 8] == value);
                };
            auto ColHasValue = [&](const int value)
                {
                    constexpr auto col = Col(INDEX);
                    return
                        (fields[col + 9 * 0] == value) ||
                        (fields[col + 9 * 1] == value) ||
                        (fields[col + 9 * 2] == value) ||
                        (fields[col + 9 * 3] == value) ||
                        (fields[col + 9 * 4] == value) ||
                        (fields[col + 9 * 5] == value) ||
                        (fields[col + 9 * 6] == value) ||
                        (fields[col + 9 * 7] == value) ||
                        (fields[col + 9 * 8] == value);
                };
            auto BoxHasValue = [&](const int value)
                {
                    constexpr auto box = Box(INDEX);
                    constexpr auto idx = (box / 3) * 27 + (box % 3) * 3;
                    return
                        (fields[idx + 0 + 0 * 9] == value) ||
                        (fields[idx + 1 + 0 * 9] == value) ||
                        (fields[idx + 2 + 0 * 9] == value) ||
                        (fields[idx + 0 + 1 * 9] == value) ||
                        (fields[idx + 1 + 1 * 9] == value) ||
                        (fields[idx + 2 + 1 * 9] == value) ||
                        (fields[idx + 0 + 2 * 9] == value) ||
                        (fields[idx + 1 + 2 * 9] == value) ||
                        (fields[idx + 2 + 2 * 9] == value);
                };
            auto SetAndSolve = [&](const int newValue)
                {
                    value = 0;
                    if (RowHasValue(newValue) ||
                        ColHasValue(newValue) ||
                        BoxHasValue(newValue))
                    {
                        return false;
                    }
                    else
                    {
                        value = newValue;
                        return Solve<INDEX + 1>(fields);
                    }
                };
            if (SetAndSolve(1) ||
                SetAndSolve(2) || 
                SetAndSolve(3) ||
                SetAndSolve(4) ||
                SetAndSolve(5) ||
                SetAndSolve(6) ||
                SetAndSolve(7) ||
                SetAndSolve(8) ||
                SetAndSolve(9) )
            {
                return true;
            }
            value = 0;
        }
        return false;
    }
    template<>
    bool Solve<81>(Fields& fields)
    {
        return true;
    }
}

bool Sudoku::Solve()
{
    return impl::Solve<0>(fields);
}

bool Sudoku::Valid() const
{
    return true;
}