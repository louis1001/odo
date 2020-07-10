
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BinOpNode final : public Node {
    Lexing::Token token;
    std::shared_ptr<Parsing::Node> left;
    std::shared_ptr<Parsing::Node> right;
    
    NodeType kind() final { return NodeType::BinOp; }

    BinOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> left_p, std::shared_ptr<Parsing::Node> right_p);

    static std::shared_ptr<Node> create(Lexing::Token token_p, std::shared_ptr<Parsing::Node> left_p, std::shared_ptr<Parsing::Node> right_p){
        return std::make_shared<BinOpNode>(std::move(token_p), std::move(left_p), std::move(right_p));
    }
};
}
