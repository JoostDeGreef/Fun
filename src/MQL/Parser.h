#pragma once

#include <vector>
#include <string>
#include <cassert>

#include "Symbol.h"

class SymbolCache
{

};

class ParserTree
{
public:
    bool Empty() const;
    void Swap(ParserTree& other);
    void Execute(SymbolCache& symbols) const;
};

class Parser
{
public:
    void Process(const std::string& input);
protected:
    // protected member functions are not part of the public API,
    // but can be covered by unit tests.
    std::vector<Symbol> ExtractSymbols(const std::string& input) const;

    std::vector<ParserTree> BuildTrees(std::vector<Symbol>& symbols) const;

private:
    SymbolCache m_symbols;
};
