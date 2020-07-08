
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ConstructorCallNode final : public Node {
    Lexing::Token t;
    
    NodeType kind() final { return NodeType::ConstructorCall; }

    explicit ConstructorCallNode(Lexing::Token t_p);
};
}
