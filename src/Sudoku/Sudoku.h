#pragma once

#include <array>
#include <string>

typedef int Field;

typedef std::array<Field, 9 * 9> Fields;

class FieldsAndCount
{
public:
    Fields fields;
    uint64_t count;
    uint64_t max_count;

    const Field& operator [] (const int index) const { return fields[index]; }
    Field& operator [] (const int index) { return fields[index]; }

    Fields::iterator begin() { return fields.begin(); }
    Fields::iterator end() { return fields.end(); }
    Fields::const_iterator begin() const { return fields.cbegin(); }
    Fields::const_iterator end() const { return fields.cend(); }
    Fields::const_iterator cbegin() const { return fields.cbegin(); }
    Fields::const_iterator cend() const { return fields.cend(); }
};

class Sudoku
{
public:
    Sudoku();

    const Field& operator () (const int index) const { return fields[index]; };
          Field& operator () (const int index)       { return fields[index]; };

    const Field& operator () (const int row, const int col) const { return fields[row * 9 + col]; };
          Field& operator () (const int row, const int col)       { return fields[row * 9 + col]; };

    bool Solve();              // fill all fields, if possible
    bool InputValid() const;   // test if there are no obvious duplicates in the input. doesn't mean it can be solved!
    void Display() const;      // create a nice ascii output
    uint64_t CountSolutions(); // count number of possible solutions

    std::string Str() const;   // create string with all values in a row
    std::string Store() const; // create a 'compressed' string from the fields
    void Load(const std::string & data); // load a compressed string into the fields
private:
    void Init();

    FieldsAndCount fields;
};
