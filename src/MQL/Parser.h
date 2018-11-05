#pragma once

#include <vector>
#include <string>
#include <cassert>

#include "Symbol.h"

class Parser
{
public:
    void Process(const std::string& input);
protected:
    // protected member functions are not part of the public API,
    // but can be covered by unit tests.
    std::vector<Symbol> ExtractSymbols(const std::string& input) const;
};
