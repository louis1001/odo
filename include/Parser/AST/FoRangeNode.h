
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FoRangeNode final : public Node {
    Lexing::Token var;
    std::shared_ptr<Parsing::Node> first;
    std::shared_ptr<Parsing::Node> second;
    std::shared_ptr<Parsing::Node> body;
    Lexing::Token rev;
    
    NodeType kind() final { return NodeType::FoRange; }

    FoRangeNode(Lexing::Token var_p, std::shared_ptr<Parsing::Node> first_p, std::shared_ptr<Parsing::Node> second_p, std::shared_ptr<Parsing::Node> body_p, Lexing::Token rev_p);

    static std::shared_ptr<Node> create(Lexing::Token var_p, std::shared_ptr<Parsing::Node> first_p, std::shared_ptr<Parsing::Node> second_p, std::shared_ptr<Parsing::Node> body_p, Lexing::Token rev_p){
        return std::make_shared<FoRangeNode>(std::move(var_p), std::move(first_p), std::move(second_p), std::move(body_p), std::move(rev_p));
    }
};
}
