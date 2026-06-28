#pragma once

#include <cassert>

#include "IOperator.h"
#include "IValue.h"

#include "OperatorRegistry.h"

namespace UnaryOperator
{
	class Plus final : public IUnaryOperator
	{
	public:
		explicit
		Plus(const IOperatorPtr& node)
			: IUnaryOperator(node)
		{}

		virtual IOperatorPtr Create(const std::vector<IOperatorPtr>& children) override
		{
			assert(children.size() == 1);
			return std::make_shared<Plus>(children.front());
		}

		virtual IValuePtr Execute() override
		{
			return m_node->Execute()->Plus();
		}

		static void Register(OperatorRegistry& registry, const int order)
		{
			registry.Register("+", 1, std::make_shared<Plus>(nullptr), order);
		}
	};

	class Minus final : public IUnaryOperator
	{
	public:
		explicit
		Minus(const IOperatorPtr& node)
			: IUnaryOperator(node)
		{}

		virtual IOperatorPtr Create(const std::vector<IOperatorPtr>& children) override
		{
			assert(children.size() == 1);
			return std::make_shared<Minus>(children.front());
		}

		virtual IValuePtr Execute() override
		{
			return m_node->Execute()->Minus();
		}

		static void Register(OperatorRegistry& registry, const int order)
		{
			registry.Register("-", 1, std::make_shared<Minus>(nullptr), order);
		}
	};

}


namespace BinaryOperator
{
	class Add final : public IBinaryOperator
	{
	public:
		Add(const IOperatorPtr& left, const IOperatorPtr& right)
			: IBinaryOperator(left, right)
		{}

		virtual IOperatorPtr Create(const std::vector<IOperatorPtr>& children) override
		{
			assert(children.size() == 2);
			return std::make_shared<Add>(children.front(),children.back());
		}

		virtual IValuePtr Execute() override
		{
			return m_left->Execute()->Add(m_right->Execute());
		}

		static void Register(OperatorRegistry& registry, const int order)
		{
			registry.Register("+", 2, std::make_shared<Add>(nullptr,nullptr), order);
		}
	};

	class Subtract final : public IBinaryOperator
	{
	public:
		Subtract(const IOperatorPtr& left, const IOperatorPtr& right)
			: IBinaryOperator(left, right)
		{}

		virtual IOperatorPtr Create(const std::vector<IOperatorPtr>& children) override
		{
			assert(children.size() == 2);
			return std::make_shared<Subtract>(children.front(), children.back());
		}

		virtual IValuePtr Execute() override
		{
			return m_left->Execute()->Subtract(m_right->Execute());
		}

		static void Register(OperatorRegistry & registry, const int order)
		{
			registry.Register("-", 2, std::make_shared<Subtract>(nullptr, nullptr), order);
		}
	};
}




namespace MultiOperator
{
	class Sum final : public IMultiOperator
	{
	public:
		Sum(const IOperatorPtr& left, const IOperatorPtr& right)
			: IMultiOperator(left, right)
		{}

		virtual IOperatorPtr Create(const std::vector<std::shared_ptr<IOperator>>& children) override
		{
			assert(children.size() >= 1);
			return std::make_shared<Sum>(children.front(), children.back());
		}

		virtual IValuePtr Execute() override
		{
			std::vector<IValuePtr> values;
			for (size_t i = 1; i < m_nodes.size(); ++i)
			{
				values.emplace_back(m_nodes[i]->Execute());
			}
			return m_nodes.front()->Execute()->Sum(values);
		}

		static void Register(OperatorRegistry & registry, const int order)
		{
			registry.Register("SUM", -1, std::make_shared<Sum>(nullptr, nullptr), order);
		}
	};
}





