
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct VarDeclarationNode final : public Node {
    std::shared_ptr<Parsing::Node> var_type;
    Lexing::Token name;
    std::shared_ptr<Parsing::Node> initial;
    
    NodeType kind() final { return NodeType::VarDeclaration; }

    VarDeclarationNode(std::shared_ptr<Parsing::Node> var_type_p, Lexing::Token name_p, std::shared_ptr<Parsing::Node> initial_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> var_type_p, Lexing::Token name_p, std::shared_ptr<Parsing::Node> initial_p){
        return std::make_shared<VarDeclarationNode>(std::move(var_type_p), std::move(name_p), std::move(initial_p));
    }
};
}
