#pragma once

#include <array>
#include <string>

typedef int Field;
typedef std::array<Field, 9 * 9> Fields;

class Sudoku
{
public:
    Sudoku();

    std::string str() const;

    const Field& operator () (const int index) const { return fields[index]; };
          Field& operator () (const int index)       { return fields[index]; };

    const Field& operator () (const int row, const int col) const { return fields[row * 9 + col]; };
          Field& operator () (const int row, const int col)       { return fields[row * 9 + col]; };

    bool Solve();
    bool Valid() const;
    void Display() const;
private:
    void Init();

    Fields fields;
};
