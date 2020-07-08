
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BoolNode : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Bool; }

    explicit BoolNode(Lexing::Token token_p);
};
}
