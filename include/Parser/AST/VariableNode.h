
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct VariableNode final : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Variable; }

    explicit VariableNode(Lexing::Token token_p);

    static std::shared_ptr<Node> create(Lexing::Token token_p){
        return std::make_shared<VariableNode>(std::move(token_p));
    }
};
}
