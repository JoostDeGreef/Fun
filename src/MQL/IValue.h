#pragma once

#include <memory>

namespace Value
{
	class Integer;
	typedef std::shared_ptr<Integer> IntegerPtr;

	class Boolean;
	typedef std::shared_ptr<Boolean> BooleanPtr;

	class String;
	typedef std::shared_ptr<String> StringPtr;
}

class IValue;
typedef std::shared_ptr<IValue> IValuePtr;

class IFunction;
typedef std::shared_ptr<IFunction> IFunctionPtr;

class IValue
{
public:
	// unary
	virtual IValuePtr Plus() = 0;
	virtual IValuePtr Minus() = 0;

	// binary
	virtual IValuePtr Add(const IValuePtr& other) = 0;
	virtual IValuePtr Subtract(const IValuePtr& other) = 0;

	// multi
	virtual IValuePtr Sum(const std::vector<IValuePtr> & values) = 0;

	virtual IFunctionPtr AddParam(const IFunctionPtr& func) = 0;
private:
};

class ValueFactory
{
public:
	static Value::IntegerPtr Integer(const int i);
	static Value::BooleanPtr Boolean(const bool b);
	static Value::StringPtr String(const std::string& s);
};
