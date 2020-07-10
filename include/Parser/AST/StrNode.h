
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct StrNode final : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Str; }

    explicit StrNode(Lexing::Token token_p);

    static std::shared_ptr<Node> create(Lexing::Token token_p){
        return std::make_shared<StrNode>(std::move(token_p));
    }
};
}
