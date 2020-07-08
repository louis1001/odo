
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FuncCallNode : public Node {
    std::shared_ptr<Parsing::Node> expr;
    Lexing::Token fname;
    std::vector<std::shared_ptr<Parsing::Node>> args;
    
    NodeType kind() final { return NodeType::FuncCall; }

    FuncCallNode(std::shared_ptr<Parsing::Node> expr_p, Lexing::Token fname_p, std::vector<std::shared_ptr<Parsing::Node>> args_p);
};
}
