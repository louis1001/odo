
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct UnaryOpNode : public Node {
    Lexing::Token token;
    std::shared_ptr<Parsing::Node> ast;
    
    NodeType kind() final { return NodeType::UnaryOp; }

    UnaryOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> ast_p);
};
}
