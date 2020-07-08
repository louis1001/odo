
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct VariableNode : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Variable; }

    explicit VariableNode(Lexing::Token token_p);
};
}
