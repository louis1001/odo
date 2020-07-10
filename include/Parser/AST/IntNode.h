
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct IntNode final : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Int; }

    explicit IntNode(Lexing::Token token_p);

    static std::shared_ptr<Node> create(Lexing::Token token_p){
        return std::make_shared<IntNode>(std::move(token_p));
    }
};
}
