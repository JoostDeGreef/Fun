#include <memory>
#include <random>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>
#include <iostream>

#include "CommonTestFunctionality.h"

class OperatorRegistryTest : public Test
{
protected:

	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
	}
};

TEST_F(OperatorRegistryTest, Init)
{


	IOperatorPtr op0 = std::make_shared<ValueOperator>(std::make_shared<Value::Integer>(1));
	IOperatorPtr op1 = std::make_shared<ValueOperator>(std::make_shared<Value::Integer>(2));
	IOperatorPtr op2 = std::make_shared<BinaryOperator::Add>(op0, op1);
	IValuePtr v = op2->Execute();
	Value::Integer* i = (Value::Integer*) & (*v);
	EXPECT_EQ(3, i->m_data);
}

