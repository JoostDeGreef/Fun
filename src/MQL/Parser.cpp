#include "Parser.h"
#include "Operator.h"

using namespace std;

void Parser::Process(const std::string& input)
{
    auto symbols = ExtractSymbols(input);
    auto trees = BuildTrees(symbols);
    for (const auto& tree : trees)
    {
        tree.Execute(m_symbols);
    }
}

enum class TextType
{
    None,
    Single,
    Double,
};

std::vector<Symbol> Parser::ExtractSymbols(const std::string& input) const
{
    std::vector<Symbol> res;
    std::string token;
    TextType text = TextType::None; 
    Operator op;
    bool escaped = false;
    auto StoreToken = [&]()
    {
        if (!token.empty())
        {
            switch (text)
            {
            default:
                assert(false);
            case TextType::None:
                res.emplace_back(Symbol::Keyword(token));
                break;
            case TextType::Single:
                res.emplace_back(Symbol::HardText(token));
                break;
            case TextType::Double:
                res.emplace_back(Symbol::SoftText(token));
                break;
            }
            token.clear();
        }
    };
    auto StoreOperator = [&]()
    {
        res.emplace_back(Symbol::Operator(op));
    };
    auto HandleText = [&](const char c,const char t)
    {
        if (escaped)
        {
            token += c;
        }
        else
        {
            if (c == t)
            {
                StoreToken();
                text = TextType::None;
            }
            else if (c == '\\')
            {
                escaped = true;
            }
            else
            {
                token += c;
            }
        }
    };
    for (const auto& c : input)
    {
        switch (text)
        {
        default:
            assert(false);

        case TextType::None:
            op = Operator::Parse(c);
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
                    text = TextType::Single;
                    break;
                case '"':
                    StoreToken();
                    text = TextType::Double;
                    break;
                default:
                    token += c;
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
    if (text != TextType::None)
    {
        // TODO: unterminated string? 
    }
    StoreToken();
    return res;
}

std::vector<ParserTree> Parser::BuildTrees(std::vector<Symbol>& symbols) const
{
    std::vector<ParserTree> trees;
    ParserTree tree;
    auto PushTree = [&trees](ParserTree& tree)
    {
        if (!tree.Empty())
        {
            trees.emplace_back();
            trees.back().Swap(tree);
        }
    };
    for (const auto& symbol : symbols)
    {

    }
    PushTree(tree);
    return trees;
}

bool ParserTree::Empty() const
{
	return false;
}

void ParserTree::Swap(ParserTree& other)
{

}

void ParserTree::Execute(SymbolCache& symbols) const
{

}
