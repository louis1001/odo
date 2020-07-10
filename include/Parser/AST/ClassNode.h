
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ClassNode final : public Node {
    Lexing::Token name;
    Lexing::Token ty;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::Class; }

    ClassNode(Lexing::Token name_p, Lexing::Token ty_p, std::shared_ptr<Parsing::Node> body_p);

    static std::shared_ptr<Node> create(Lexing::Token name_p, Lexing::Token ty_p, std::shared_ptr<Parsing::Node> body_p){
        return std::make_shared<ClassNode>(std::move(name_p), std::move(ty_p), std::move(body_p));
    }
};
}
