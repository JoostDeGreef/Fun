#include <cstdlib>
#include <iostream>

#include "Sudoku.h"

int main()
{
    Sudoku s;
    s(0, 1) = 2;
    s(1, 1) = 2;
    std::cout << s.str() << std::endl;
    if (s.Valid())
    {
        std::cout << "Should be invalid!" << std::endl;
    }
    s(0, 1) = 0;
    if (s.Solve())
    {
        s.Display();
    }
    else
    {
        std::cout << "This should be solvable!" << std::endl;
    }
    return EXIT_SUCCESS;
}
