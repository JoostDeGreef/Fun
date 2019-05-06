#pragma once

#include <vector>
#include <string>
#include <cassert>

class Operator
{
public:
    enum class Value
    {
        Uninitialized,

        Invalid,

        Assign,            //  =
        Equal,             //  ==
        Plus,              //  +
        Increment,         //  ++
        PlusAssign,        //  +=
        Minus,             //  -
        Decrement,         //  --
        MinusAssign,       //  -=
        Not,               //  !
        NotEqual,          //  !=
        Comma,             //  ,
        SemiComma,         //  ; 
        Greater,           //  >
        GreaterOrEqual,    //  >=
        Less,              //  <
        LessOrEqual,       //  <=
        And,               //  &
        AndAssign,         //  &=
        LogicalAnd,        //  &&
        LogicalAndAssign,  //  &&=
        Or,                //  |
        OrAssign,          //  |=
        LogicalOr,         //  ||
        LogicalOrAssign,   //  ||=
    };

    Operator()
        : m_value(Value::Uninitialized)
    {}
    //explicit 
    Operator(const Value value)
        : m_value(value)
    {}
    Operator(const Operator& other)
        : m_value(other.m_value)
    {}

    Operator& operator = (const Operator& other) { m_value = other.m_value; return *this; }
    Operator& operator = (const Value value) { m_value = value; return *this; }

    bool operator == (const Operator& other) const { return m_value == other.m_value; }
    bool operator != (const Operator& other) const { return m_value != other.m_value; }

    bool IsValid() const { return m_value != Value::Uninitialized && m_value != Value::Invalid; }

    const Value& GetValue() const { return m_value; }
    const std::string GetText() const;

    static Operator Parse(const std::string& input);
    static Operator Parse(const std::string::value_type input);
protected:
private:
    Value m_value;
};

