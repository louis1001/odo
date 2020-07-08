
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BinOpNode : public Node {
    Lexing::Token token;
    std::shared_ptr<Parsing::Node> ast;
    std::shared_ptr<Parsing::Node> ast1;
    
    NodeType kind() final { return NodeType::BinOp; }

    BinOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> ast_p, std::shared_ptr<Parsing::Node> ast1_p);
};
}
