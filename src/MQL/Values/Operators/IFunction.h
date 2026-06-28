#pragma once

class IFunction
{
public:
	virtual IFunctionPtr AddParam(const Value::Boolean& value) = 0;
	virtual IFunctionPtr AddParam(const Value::Integer& value) = 0;
	virtual IFunctionPtr AddParam(const Value::String& value) = 0;
	virtual IValuePtr Execute() = 0;
};

template<template<typename TF> class FUNC, typename T>
class TFunction : public IFunction, public std::enable_shared_from_this<FUNC<T>>
{
public:
	typedef std::shared_ptr<FUNC<T>> FuncPtr;
	typedef std::shared_ptr<T> TPtr;

	static auto Create(const T& value)
	{
		auto res = std::make_shared<FUNC<T>>();
		res->m_result = std::make_shared<T>(value);
		return res;
	}

	virtual IFunctionPtr AddParam(const Value::Boolean& /*value*/) override
	{
		// report error!
		return shared_from_this();
	}
	virtual IFunctionPtr AddParam(const Value::Integer& /*value*/) override
	{
		// report error!
		return shared_from_this();
	}
	virtual IFunctionPtr AddParam(const Value::String& /*value*/) override
	{
		// report error!
		return shared_from_this();
	}
	virtual IValuePtr Execute() override
	{
		return m_result;
	}

protected:
	TPtr m_result;
};