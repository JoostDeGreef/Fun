#pragma once

namespace Value
{
	namespace Operator
	{
		template<typename T>
		class Add final : public TFunction<Add,T>
		{
		public:
		};

		template<>
		class Add<Value::Integer> final : public TFunction<Add,Value::Integer>
		{
		public:
			virtual IFunctionPtr AddParam(const Value::Integer& value) override
			{
				m_result->m_data += value.m_data;
				return shared_from_this();
			}
		};
		
		template<>
		class Add<Value::Boolean> final : public TFunction<Add, Value::Boolean>
		{
		public:
		};
		
		template<>
		class Add<Value::String> final : public TFunction<Add, Value::String>
		{
		public:
			virtual IFunctionPtr AddParam(const Value::String& value) override
			{
				m_result->m_data += value.m_data;
				return shared_from_this();
			}
		};
	}
}
