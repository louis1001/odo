
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FuncCallNode final : public Node {
    std::shared_ptr<Parsing::Node> expr;
    Lexing::Token fname;
    std::vector<std::shared_ptr<Parsing::Node>> args;
    
    NodeType kind() final { return NodeType::FuncCall; }

    FuncCallNode(std::shared_ptr<Parsing::Node> expr_p, Lexing::Token fname_p, std::vector<std::shared_ptr<Parsing::Node>> args_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> expr_p, Lexing::Token fname_p, std::vector<std::shared_ptr<Parsing::Node>> args_p){
        return std::make_shared<FuncCallNode>(std::move(expr_p), std::move(fname_p), std::move(args_p));
    }
};
}
