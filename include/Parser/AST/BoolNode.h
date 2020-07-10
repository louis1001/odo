
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BoolNode final : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Bool; }

    explicit BoolNode(Lexing::Token token_p);

    static std::shared_ptr<Node> create(Lexing::Token token_p){
        return std::make_shared<BoolNode>(std::move(token_p));
    }
};
}
