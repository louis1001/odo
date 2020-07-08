
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BinOpNode : public Node {
    Lexing::Token token;
    std::shared_ptr<Parsing::Node> left;
    std::shared_ptr<Parsing::Node> right;
    
    NodeType kind() final { return NodeType::BinOp; }

    BinOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> left_p, std::shared_ptr<Parsing::Node> right_p);
};
}
