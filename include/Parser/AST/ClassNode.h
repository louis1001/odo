
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ClassNode final : public Node {
    Lexing::Token name;
    Lexing::Token ty;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::Class; }

    ClassNode(Lexing::Token name_p, Lexing::Token ty_p, std::shared_ptr<Parsing::Node> body_p);
};
}
