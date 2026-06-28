#include "Parser.h"
#include "Operator.h"

using namespace std;

Parser::Parser(OperatorRegistry & operatorRegistry)
	: m_operatorRegistry(operatorRegistry)
	, m_operatorStack()
	, m_expressionStack()
	, m_tokens(m_operatorRegistry.GetRegisteredTokens())
    , m_token()
    , m_text(TextType::None)
    , m_escaped(false)
{}

void Parser::HandleText(const char c, const char t)
{
	if (m_escaped)
	{
		m_token += c;
	}
	else
	{
		if (c == t)
		{
			m_expressionStack.push(std::make_shared<ValueOperator>(ValueFactory::String(m_token)));
			m_text = TextType::None;
			m_token.clear();
		}
		else if (c == '\\')
		{
			m_escaped = true;
		}
		else
		{
			m_token += c;
		}
	}
};

std::vector<IOperatorPtr> Parser::Parse(const std::string& input)
{
	std::vector<IOperatorPtr> res;
	for (const char c : input)
	{
        switch (m_text)
        {
        default:
            assert(false);

        case TextType::None:
            auto op = Operator::Parse(c);
            if (op.IsValid())
            {
                StoreToken();
                StoreOperator();
            }
            else
            {
                switch (c)
                {
                case '\r':
                case '\n':
                case '\t':
                case ' ':
                    StoreToken();
                    break;
                case '\'':
                    StoreToken();
                    m_text = TextType::Single;
                    break;
                case '"':
                    StoreToken();
                    m_text = TextType::Double;
                    break;
                default:
                    m_token += c;
                    break;
                }
            }
            break;
        
        case TextType::Single:
            HandleText(c,'\'');
            break;
        
        case TextType::Double:
            HandleText(c,'"');
            break;        
        }
    }
    if (m_text != TextType::None)
    {
        // TODO: unterminated string? 
    }
	return res;
}









//
//
//
//
//void Parser::Process(const std::string& input)
//{
//    auto symbols = ExtractSymbols(input);
//    auto trees = BuildTrees(symbols);
//    for (const auto& tree : trees)
//    {
//        tree.Execute(m_symbols);
//    }
//}
//
//
//std::vector<Symbol> Parser::ExtractSymbols(const std::string& input) const
//{
//    std::vector<Symbol> res;
//    std::string token;
//    TextType text = TextType::None; 
//    Operator op;
//    bool escaped = false;
//    auto StoreToken = [&]()
//    {
//        if (!token.empty())
//        {
//            switch (text)
//            {
//            default:
//                assert(false);
//            case TextType::None:
//                res.emplace_back(Symbol::Keyword(token));
//                break;
//            case TextType::Single:
//                res.emplace_back(Symbol::HardText(token));
//                break;
//            case TextType::Double:
//                res.emplace_back(Symbol::SoftText(token));
//                break;
//            }
//            token.clear();
//        }
//    };
//    auto StoreOperator = [&]()
//    {
//        res.emplace_back(Symbol::Operator(op));
//    };
//    auto HandleText = [&](const char c,const char t)
//    {
//        if (escaped)
//        {
//            token += c;
//        }
//        else
//        {
//            if (c == t)
//            {
//                StoreToken();
//                text = TextType::None;
//            }
//            else if (c == '\\')
//            {
//                escaped = true;
//            }
//            else
//            {
//                token += c;
//            }
//        }
//    };
//    for (const auto& c : input)
//    {
//        switch (text)
//        {
//        default:
//            assert(false);
//
//        case TextType::None:
//            op = Operator::Parse(c);
//            if (op.IsValid())
//            {
//                StoreToken();
//                StoreOperator();
//            }
//            else
//            {
//                switch (c)
//                {
//                case '\r':
//                case '\n':
//                case '\t':
//                case ' ':
//                    StoreToken();
//                    break;
//                case '\'':
//                    StoreToken();
//                    text = TextType::Single;
//                    break;
//                case '"':
//                    StoreToken();
//                    text = TextType::Double;
//                    break;
//                default:
//                    token += c;
//                    break;
//                }
//            }
//            break;
//        
//        case TextType::Single:
//            HandleText(c,'\'');
//            break;
//        
//        case TextType::Double:
//            HandleText(c,'"');
//            break;        
//        }
//    }
//    if (text != TextType::None)
//    {
//        // TODO: unterminated string? 
//    }
//    StoreToken();
//    return res;
//}
//
//std::vector<ParserTree> Parser::BuildTrees(std::vector<Symbol>& symbols) const
//{
//    std::vector<ParserTree> trees;
//    ParserTree tree;
//    auto PushTree = [&trees](ParserTree& tree)
//    {
//        if (!tree.Empty())
//        {
//            trees.emplace_back();
//            trees.back().Swap(tree);
//        }
//    };
//    for (const auto& symbol : symbols)
//    {
//
//    }
//    PushTree(tree);
//    return trees;
//}
//
//bool ParserTree::Empty() const
//{
//	return false;
//}
//
//void ParserTree::Swap(ParserTree& other)
//{
//
//}
//
//void ParserTree::Execute(SymbolCache& symbols) const
//{
//
//}
