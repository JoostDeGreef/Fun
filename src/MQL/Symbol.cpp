#include "Symbol.h"

using namespace std;

Symbol::~Symbol()
{
    Clear();
}
Symbol::Symbol()
    : m_type(Type::Uninitialized)
{}
Symbol::Symbol(const Symbol& other)
    : Symbol()
{
    *this = other;
}
Symbol::Symbol(Symbol&& other)
    : Symbol()
{
    *this = other;
}
Symbol & Symbol::operator = (const Symbol& other)
{
    Clear();
    switch (other.m_type)
    {
    case Type::Uninitialized:
        break;
    case Type::HardText:
    case Type::SoftText:
    case Type::Keyword:
        new (&m_string) string(other.m_string);
        break;
    case Type::Operator:
        new (&m_operator) ::Operator(other.m_operator);
        break;
    default:
        assert(false);
        break;
    }
    m_type = other.m_type;
    return *this;
}
Symbol & Symbol::operator = (Symbol&& other)
{
    Clear();
    switch (other.m_type)
    {
    case Type::Uninitialized:
        break;
    case Type::HardText:
    case Type::SoftText:
    case Type::Keyword:
        new (&m_string) string(std::move(other.m_string));
        break;
    case Type::Operator:
        new (&m_operator) ::Operator(std::move(other.m_operator));
        break;
    default:
        assert(false);
        break;
    }
    m_type = other.m_type;
    other.m_type = Type::Uninitialized;
    return *this;
}
void Symbol::Clear()
{
    switch (m_type)
    {
    case Type::Uninitialized:
        break;
    case Type::HardText:
    case Type::SoftText:
    case Type::Keyword:
        (&m_string)->~string();
        break;
    case Type::Operator:
        (&m_operator)->~Operator();
        break;
    default:
        assert(false);
        break;
    }
    m_type = Type::Uninitialized;
}

bool Symbol::operator == (const Symbol& other) const
{
    if (m_type == other.m_type)
    {
        switch (m_type)
        {
        case Type::Uninitialized:
            return true;
        case Type::HardText:
        case Type::SoftText:
        case Type::Keyword:
            return m_string == other.m_string;
        case Type::Operator:
            return m_operator == other.m_operator;
        default:
            assert(false);
            break;
        }
    }
    return false;
}
bool Symbol::operator != (const Symbol& other) const
{
    return !this->operator==(other);
}

Symbol Symbol::Uninitialized()
{
    return Symbol();
}

Symbol Symbol::Keyword(const std::string& keyword)
{
    Symbol s;
    s.m_type = Type::Keyword;
    new (&s.m_string) string(keyword);
    return s;
}

Symbol Symbol::Operator(const ::Operator& op)
{
    Symbol s;
    s.m_type = Type::Operator;
    new (&s.m_operator) ::Operator(op);
    return s;
}

Symbol Symbol::HardText(const std::string& text)
{
    Symbol s;
    s.m_type = Type::HardText;
    new (&s.m_string) string(text);
    return s;
}

Symbol Symbol::SoftText(const std::string& text)
{
    Symbol s;
    s.m_type = Type::SoftText;
    new (&s.m_string) string(text);
    return s;
}


