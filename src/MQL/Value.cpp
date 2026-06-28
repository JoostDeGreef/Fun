#include <vector>

#include "Value.h"


Value::IntegerPtr ValueFactory::Integer(const int i)
{
	return std::make_shared<Value::Integer>(i);
}

Value::BooleanPtr ValueFactory::Boolean(const bool b)
{
	return std::make_shared<Value::Boolean>(b);
}

Value::StringPtr ValueFactory::String(const std::string& s)
{
	return std::make_shared<Value::String>(s);
}



