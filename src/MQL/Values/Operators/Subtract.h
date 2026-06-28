#pragma once

namespace Value
{
	namespace Operator
	{
		template<typename T>
		class Subtract final : public TFunction<Plus, T>
		{
		public:
		};
		template<>
		class Subtract<Value::Integer> final : public TFunction<Plus, Value::Integer>
		{
		public:
			virtual IFunctionPtr AddParam(const Value::Integer& value) override
			{
				m_result->m_data -= value.m_data;
				return shared_from_this();
			}
		};
		template<>
		class Subtract<Value::Boolean> final : public TFunction<Plus, Value::Boolean>
		{};
	}
}

