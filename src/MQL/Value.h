#pragma once

#include <memory>
#include <string>

#include "IValue.h"
#include "Values/Operators/IFunction.h"

namespace Value
{
	template<typename T>
	class TValue : public IValue, std::enable_shared_from_this<T>
	{
	private:
		T* THIS()
		{
			return static_cast<T*>(this);
		}
	public:
		// unary
		IValuePtr Plus() override;
		IValuePtr Minus() override;

		// binary
		IValuePtr Add(const IValuePtr& other) override;
		IValuePtr Subtract(const IValuePtr& other) override;

		// multi
		IValuePtr Sum(const std::vector<IValuePtr>& values) override;

		IFunctionPtr AddParam(const IFunctionPtr& func) override;
	};

	class Boolean final : public TValue<Value::Boolean>
	{
	public:
		Boolean()
			: m_data(false)
		{}
		explicit
		Boolean(const bool b)
			: m_data(b)
		{}
		Boolean(const Boolean& other)
			: m_data(other.m_data)
		{}

		bool m_data;
	};

	class Integer final : public TValue<Value::Integer>
	{
	public:
		Integer()
			: m_data(0)
		{}
		explicit
		Integer(const int i)
			: m_data(i)
		{}
		Integer(const Integer& other)
			: m_data(other.m_data)
		{}

		int m_data;
	};

	class String final : public TValue<Value::String>
	{
	public:
		String()
			: m_data("")
		{}
		explicit
		String(const std::string & s)
			: m_data(s)
		{}
		String(const String& other)
			: m_data(other.m_data)
		{}

		std::string m_data;
	};
}

#include "Values/Operators/Add.h"
#include "Values/Operators/Plus.h"
#include "Values/Operators/Subtract.h"
#include "Values/Operators/Minus.h"


namespace Value
{
	template<typename T>
	IValuePtr TValue<T>::Plus()
	{
		return Value::Operator::Plus<T>::Create(*THIS())->Execute();
	}
	template<typename T>
	IValuePtr TValue<T>::Minus()
	{
		return Value::Operator::Minus<T>::Create(*THIS())->Execute();
	}
	template<typename T>
	IValuePtr TValue<T>::Add(const IValuePtr& other)
	{
		auto op = Value::Operator::Add<T>::Create(*THIS());
		return other->AddParam(op)->Execute();
	}
	template<typename T>
	IValuePtr TValue<T>::Subtract(const IValuePtr& other)
	{
		auto op = Value::Operator::Subtract<T>::Create(*THIS());
		return other->AddParam(op)->Execute();
	}
	template<typename T>
	IValuePtr TValue<T>::Sum(const std::vector<IValuePtr>& values)
	{
		IFunctionPtr op = Value::Operator::Add<T>::Create(*THIS());
		for (const IValuePtr& value : values)
		{
			op = value->AddParam(op);
		}
		return op->Execute();
	}
	template<typename T>
	IFunctionPtr TValue<T>::AddParam(const IFunctionPtr& func)
	{
		return func->AddParam(*THIS());
	}
}


