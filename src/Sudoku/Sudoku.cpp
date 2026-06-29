#include "Sudoku.h"
#include "SudokuCompressor.h"

#include <iostream>
#include <format>
#include <ranges>

template<int INDEX>
struct Kernel
{
    static constexpr int row = INDEX / 9;
    static constexpr int col = INDEX % 9;
    static constexpr int box = (row / 3) * 3 + (col / 3);

    static inline int RowCountValue(const FieldsAndCount& fields, const int value)
    {
        return
            (fields[row * 9 + 0] == value ? 1 : 0) +
            (fields[row * 9 + 1] == value ? 1 : 0) +
            (fields[row * 9 + 2] == value ? 1 : 0) +
            (fields[row * 9 + 3] == value ? 1 : 0) +
            (fields[row * 9 + 4] == value ? 1 : 0) +
            (fields[row * 9 + 5] == value ? 1 : 0) +
            (fields[row * 9 + 6] == value ? 1 : 0) +
            (fields[row * 9 + 7] == value ? 1 : 0) +
            (fields[row * 9 + 8] == value ? 1 : 0);
    }
    static inline bool RowHasValue(const FieldsAndCount& fields, const int value)
    {
        return RowCountValue(fields, value) != 0;
    }

    static inline int ColCountValue(const FieldsAndCount& fields, const int value)
    {
        return
            (fields[col + 9 * 0] == value ? 1 : 0) +
            (fields[col + 9 * 1] == value ? 1 : 0) +
            (fields[col + 9 * 2] == value ? 1 : 0) +
            (fields[col + 9 * 3] == value ? 1 : 0) +
            (fields[col + 9 * 4] == value ? 1 : 0) +
            (fields[col + 9 * 5] == value ? 1 : 0) +
            (fields[col + 9 * 6] == value ? 1 : 0) +
            (fields[col + 9 * 7] == value ? 1 : 0) +
            (fields[col + 9 * 8] == value ? 1 : 0);
    }
    static inline bool ColHasValue(const FieldsAndCount& fields, const int value)
    {
        return ColCountValue(fields, value) != 0;
    }

    static inline int BoxCountValue(const FieldsAndCount& fields, const int value)
    {
        constexpr auto idx = (box / 3) * 27 + (box % 3) * 3;
        return
            (fields[idx + 0 + 0 * 9] == value ? 1 : 0) +
            (fields[idx + 1 + 0 * 9] == value ? 1 : 0) +
            (fields[idx + 2 + 0 * 9] == value ? 1 : 0) +
            (fields[idx + 0 + 1 * 9] == value ? 1 : 0) +
            (fields[idx + 1 + 1 * 9] == value ? 1 : 0) +
            (fields[idx + 2 + 1 * 9] == value ? 1 : 0) +
            (fields[idx + 0 + 2 * 9] == value ? 1 : 0) +
            (fields[idx + 1 + 2 * 9] == value ? 1 : 0) +
            (fields[idx + 2 + 2 * 9] == value ? 1 : 0);
    }
    static inline bool BoxHasValue(const FieldsAndCount& fields, const int value)
    {
        return BoxCountValue(fields, value) != 0;
    }

    template<int VALUE>
    static bool SetAndSolve(FieldsAndCount& fields)
    {
        int& value = fields[INDEX];
        value = 0;
        if (RowHasValue(fields, VALUE) ||
            ColHasValue(fields, VALUE) ||
            BoxHasValue(fields, VALUE))
        {
            return false;
        }
        else
        {
            value = VALUE;
            return Kernel<INDEX + 1>::Solve(fields);
        }
    }

    static bool Solve(FieldsAndCount& fields)
    {
        int& value = fields[INDEX];
        if (value)
        {
            return Kernel<INDEX + 1>::Solve(fields);
        }
        else
        {
            if (SetAndSolve<1>(fields) ||
                SetAndSolve<2>(fields) ||
                SetAndSolve<3>(fields) ||
                SetAndSolve<4>(fields) ||
                SetAndSolve<5>(fields) ||
                SetAndSolve<6>(fields) ||
                SetAndSolve<7>(fields) ||
                SetAndSolve<8>(fields) ||
                SetAndSolve<9>(fields))
            {
                return true;
            }
            else
            {
                value = 0;
                return false;
            }
        }
    }
    static bool InputValid(const FieldsAndCount& fields)
    {
        const int& value = fields[INDEX];
        if (value)
        {
            if (RowCountValue(fields, value) != 1 ||
                ColCountValue(fields, value) != 1 ||
                BoxCountValue(fields, value) != 1)
            {
                return false;
            }
        }
        return Kernel<INDEX + 1>::InputValid(fields);
    }
};
template<>
struct Kernel<81>
{
    static bool Solve(FieldsAndCount& fields)
    {
        fields.count++;
        return fields.count == fields.max_count;
    }
    static bool InputValid(const FieldsAndCount& fields)
    {
        return true;
    }
};

Sudoku::Sudoku()
{
    Init();
}

std::string Sudoku::Str() const
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
    fields.fields.fill(0);
}

bool Sudoku::Solve()
{
    fields.count = 0;
    fields.max_count = 1;
    return Kernel<0>::Solve(fields);
}

uint64_t Sudoku::CountSolutions()
{
    fields.count = 0;
    fields.max_count = 100;
    Kernel<0>::Solve(fields);
    return fields.count;
}

bool Sudoku::InputValid() const
{
    return Kernel<0>::InputValid(fields);
}

std::string Sudoku::Store() const
{
    SudokuCompressor sc;
    std::string output;
    for (int i = 0; i < 81; ++i)
    {
        sc.PushDecimalDigit(fields[i]);
    }
    for (int i = 0; i < 45; ++i)
    {
        output.push_back(sc.PopBase64Char());
    }
    return output;
}

void Sudoku::Load(const std::string& data)
{
    SudokuCompressor sc;
    for (const char c : data | std::views::reverse)
    {
        sc.Pushbase64Char(c);
    }
    for (int i = 0; i < 81; ++i)
    {
        fields[80 - i] = sc.PopDecimalDigit();
    }
}
