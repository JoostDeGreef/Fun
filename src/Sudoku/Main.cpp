#include <cstdlib>
#include <iostream>

#include "Sudoku.h"
#include "SudokuCompressor.h"

void TestSudoku()
{
    Sudoku s;
    s(0, 1) = 2;
    s(1, 1) = 2;
    if (s.InputValid())
    {
        std::cout << s.Str() << std::endl;
        std::cout << "Should be invalid!" << std::endl;
    }
    s(0, 1) = 0;
    if (!s.InputValid())
    {
        std::cout << s.Str() << std::endl;
        std::cout << "Should be valid!" << std::endl;
    }
    uint64_t count = s.CountSolutions();
    s.Display();
    std::cout << "has " << count << " solutions" << std::endl;
    if (s.Solve())
    {
        s.Display();
    }
    else
    {
        std::cout << "This should be solvable!" << std::endl;
    }
    std::cout << s.Str() << std::endl;
    std::string temp = s.Store();
    std::cout << temp << std::endl;
    s.Load(temp);
    s.Display();
}

void TestSudokuCompressor()
{
    SudokuCompressor sc;
    for (int i = 0; i < 60; ++i)
    {
        int j = 9;
        std::cout << j;
        sc.PushDecimalDigit(j);
    }
    std::cout << std::endl;
    for (int i = 0; i < 60; ++i)
    {
        int j = sc.PopDecimalDigit();
        std::cout << j;
    }
    std::cout << std::endl;
    sc.Clear();
    for (int i = 0; i < 50; ++i)
    {
        char c = '_';
        std::cout << c;
        sc.Pushbase64Char(c);
    }
    std::cout << std::endl;
    for (int i = 0; i < 50; ++i)
    {
        char c = sc.PopBase64Char();
        std::cout << c;
    }
    std::cout << std::endl;
}

int main()
{
    //TestSudokuCompressor();
    TestSudoku();
    return EXIT_SUCCESS;
}
