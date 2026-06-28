#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <stack>

#include "OperatorRegistry.h"

class Parser
{
protected:
	enum class TextType
	{
		None,
		Single,
		Double,
	};

public:
	explicit
	Parser(OperatorRegistry & operatorRegistry);
	
	std::vector<IOperatorPtr> Parse(const std::string& input);

protected:
	void HandleText(const char c, const char t);

private:
	OperatorRegistry& m_operatorRegistry;
	std::stack<IOperatorPtr> m_operatorStack;
	std::stack<IOperatorPtr> m_expressionStack;

	// parser state
	OperatorRegistry::Tokens m_tokens;
	std::string m_token;
	TextType m_text;
	bool m_escaped;
};
