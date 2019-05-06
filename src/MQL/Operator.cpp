#include <map>

#include "Operator.h"
#include "grid.h"

using namespace std;

namespace
{
    using Value = Operator::Value;

    grid<std::string, Value> operators =
    {
        { "=",   Value::Assign           },
        { "==",  Value::Equal            },
        { "+",   Value::Plus             },
        { "++",  Value::Increment        },
        { "+=",  Value::PlusAssign       },
        { "-",   Value::Minus            },
        { "--",  Value::Decrement        },
        { "-=",  Value::MinusAssign      },
        { "!",   Value::Not              },
        { "!=",  Value::NotEqual         },
        { ",",   Value::Comma            },
        { ";",   Value::SemiComma        },
        { ">",   Value::Greater          },
        { ">=",  Value::GreaterOrEqual   },
        { "<",   Value::Less             },
        { "<=",  Value::LessOrEqual      },
        { "&",   Value::And              },
        { "&=",  Value::AndAssign        },
        { "&&",  Value::LogicalAnd       },
        { "&&=", Value::LogicalAndAssign },
        { "|",   Value::Or               },
        { "|=",  Value::OrAssign         },
        { "||",  Value::LogicalOr        },
        { "||=", Value::LogicalOrAssign  },
    };
}

Operator Operator::Parse(const std::string& input)
{
    Value value = Value::Invalid;
    auto iter = operators.find<0>(input);
    if (iter)
    {
        value = iter.get_field<1>();
    }
    return Operator{ value };
}
Operator Operator::Parse(const std::string::value_type input)
{
    return Parse(std::string(1, input));
}

const std::string Operator::GetText() const
{
    std::string op = "";
    auto iter = operators.find<1>(m_value);
    if (iter)
    {
        op = iter.get_field<0>();
    }
    return op;
}
