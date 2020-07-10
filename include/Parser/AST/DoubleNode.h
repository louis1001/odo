
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct DoubleNode final : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Double; }

    explicit DoubleNode(Lexing::Token token_p);

    static std::shared_ptr<Node> create(Lexing::Token token_p){
        return std::make_shared<DoubleNode>(std::move(token_p));
    }
};
}
