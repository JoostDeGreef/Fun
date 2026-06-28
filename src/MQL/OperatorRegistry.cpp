#include <utility>
#include <tuple>

#include "Operator.h"
#include "OperatorRegistry.h"

using namespace std;

OperatorRegistry::OperatorRegistry()
{
	//1 	() 	Left to Right
	//2	[] 	Left to Right
	//3	.	Left to Right
	//4	->	Left to Right
	//5 	++ --	Left to Right
	//6 	++ --	Right to Left
	//7	+	-	Right to Left
	//8 	!	~	Right to Left
	//9 	(type)	Right to Left
	//10	*	Right to Left
	//11	&	Right to Left
	//12 	sizeof 	Right to Left
	//13	* / %	Left to Right
	//14	+ -		Left to Right
	//15 	<<  >> 	Left to Right
	//16	< <= Left to Right
	//17	> >= Left to Right
	//18 	==  != 	Left to Right
	//19	& Left to Right
	//20	^ Left to Right
	//21	| Left to Right
	//22	&& Left to Right
	//23 	| | 	Left to Right
	//24	? : Right to Left
	//25	= Right to Left
	//26	+= -= Right to Left
	//27	*= /= Right to Left
	//28	%= &= Right to Left

	//100 any function
		
	UnaryOperator::Plus::Register(*this,7);
	UnaryOperator::Minus::Register(*this,7);

	BinaryOperator::Add::Register(*this,14);
	BinaryOperator::Subtract::Register(*this,14);

	MultiOperator::Sum::Register(*this,100);
}

void OperatorRegistry::Register(const std::string& op, const int argc, const IOperatorPtr & operatorPtr, const int order)
{
	m_operators.insert(op, argc, order, operatorPtr);
}

OperatorRegistry::Tokens OperatorRegistry::GetRegisteredTokens()
{
	Tokens res;
	for (const auto& key : m_operators)
	{
		res.emplace_back(key);
	}
	return res;
}
