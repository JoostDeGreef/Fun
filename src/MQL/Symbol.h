#pragma once

#include <vector>
#include <string>
#include <cassert>

#include "Operator.h"

class Symbol
{
public:
    enum class Type
    {
        Uninitialized,
        Keyword,
        Operator,
        HardText, // 'text'
        SoftText, // "text"
    };

    ~Symbol();
    Symbol();
    Symbol(const Symbol& other);
    Symbol(Symbol&& other);
    Symbol & operator = (const Symbol& other);
    Symbol & operator = (Symbol&& other);

    bool operator == (const Symbol& other) const;
    bool operator != (const Symbol& other) const;

    static Symbol Uninitialized();
    static Symbol Keyword(const std::string& keyword);
    static Symbol Operator(const Operator& op);
    static Symbol HardText(const std::string& text);
    static Symbol SoftText(const std::string& text);

    Type GetType() const { return m_type; }

    bool IsUninitialized() const { return m_type == Type::Uninitialized; }
    bool IsKeyword() const { return m_type == Type::Keyword; }
    bool IsOperator() const { return m_type == Type::Operator; }
    bool IsText() const { return IsHardText() || IsSoftText(); }
    bool IsHardText() const { return m_type == Type::HardText; }
    bool IsSoftText() const { return m_type == Type::SoftText; }

    const std::string& GetKeyword() const { assert(IsKeyword()); return m_string; }
    const ::Operator& GetOperator() const { assert(IsOperator()); return m_operator; }
    const std::string& GetText() const { assert(IsText()); return m_string; }
    const std::string& GetHardText() const { assert(IsHardText()); return m_string; }
    const std::string& GetSoftText() const { assert(IsSoftText()); return m_string; }

protected:
    void Clear();
private:
    Type m_type;
    union
    {
        std::string m_string;
        ::Operator m_operator;
    };
};

