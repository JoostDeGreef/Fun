#pragma once

#include <map>
#include <unordered_set>
#include <string>
#include <tuple>

#include "grid.h"
#include "IOperator.h"

class OperatorRegistry
{
public:
	//           name         argc order
	typedef grid<std::string, int, int, IOperatorPtr> Operators;
	typedef std::vector<Operators::row_type> Tokens;

	OperatorRegistry();

	void Register(const std::string & op, const int argc, const IOperatorPtr & operatorPtr, const int order);

	Tokens GetRegisteredTokens();
private:
	Operators m_operators;
};

