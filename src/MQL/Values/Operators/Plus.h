#pragma once

namespace Value
{
	namespace Operator
	{
		template<typename T>
		class Plus final : public TFunction<Plus, T>
		{};
		template<>
		class Plus<Value::Integer> final : public TFunction<Plus, Value::Integer>
		{};
		template<>
		class Plus<Value::Boolean> final : public TFunction<Plus, Value::Boolean>
		{};
	}
}
