
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct StrNode : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Str; }

    explicit StrNode(Lexing::Token token_p);
};
}
