#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <memory>

#include "IValue.h"

class IOperator;
typedef std::shared_ptr<IOperator> IOperatorPtr;

class IOperator
{
public:
	virtual IValuePtr Execute() = 0;
	virtual IOperatorPtr Create(const std::vector<IOperatorPtr> & children) = 0;
protected:
private:
};

class ValueOperator : public IOperator
{
public:
	explicit
	ValueOperator(const IValuePtr& value)
		: m_value(value)
	{}
	virtual IValuePtr Execute() override
	{
		return m_value;
	}
	virtual IOperatorPtr Create(const std::vector<IOperatorPtr> & children) override
	{
		assert(children.empty());
		return std::make_shared<ValueOperator>(nullptr);
	}
protected:
	IValuePtr m_value;
};

class IUnaryOperator : public IOperator
{
public:
	explicit
    IUnaryOperator(const IOperatorPtr& node)
		: m_node(node)
	{}
protected:
	IOperatorPtr m_node;
};

class IBinaryOperator : public IOperator
{
public:
	IBinaryOperator(const IOperatorPtr& left, const IOperatorPtr& right)
		: m_left(left)
		, m_right(right)
	{}
protected:
	IOperatorPtr m_left;
	IOperatorPtr m_right;
};

class IMultiOperator : public IOperator
{
public:
	template<typename... NODES>
	IMultiOperator(const IOperatorPtr & node, const NODES & ... nodes)
		: m_nodes({ node, nodes... })
	{}
protected:
	std::vector<std::shared_ptr<IOperator>> m_nodes;
};

