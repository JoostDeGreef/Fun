#pragma once

namespace Value
{
	namespace Operator
	{
		template<typename T>
		class Minus final : public TFunction<Minus,T>
		{};
		template<>
		class Minus<Value::Integer> final : public TFunction<Minus, Value::Integer>
		{
		public:
			virtual IValuePtr Execute() override
			{
				m_result->m_data = -m_result->m_data;
				return m_result;
			}
		};
		template<>
		class Minus<Value::Boolean> final : public TFunction<Minus, Value::Boolean>
		{};
	}
}
