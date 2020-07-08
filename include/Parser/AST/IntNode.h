
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct IntNode final : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Int; }

    explicit IntNode(Lexing::Token token_p);
};
}
