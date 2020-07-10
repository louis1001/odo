
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct EnumNode final : public Node {
    Lexing::Token name;
    std::vector<std::shared_ptr<Parsing::Node>> variants;
    
    NodeType kind() final { return NodeType::Enum; }

    EnumNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> variants_p);

    static std::shared_ptr<Node> create(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> variants_p){
        return std::make_shared<EnumNode>(std::move(name_p), std::move(variants_p));
    }
};
}
