
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct MemberVarNode final : public Node {
    std::shared_ptr<Parsing::Node> inst;
    Lexing::Token name;
    
    NodeType kind() final { return NodeType::MemberVar; }

    MemberVarNode(std::shared_ptr<Parsing::Node> inst_p, Lexing::Token name_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> inst_p, Lexing::Token name_p){
        return std::make_shared<MemberVarNode>(std::move(inst_p), std::move(name_p));
    }
};
}
