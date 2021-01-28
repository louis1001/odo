
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ListDeclarationNode final : public Node {
    std::shared_ptr<Parsing::Node> var_type;
    Lexing::Token name;
    int dim;
    std::shared_ptr<Parsing::Node> initial;
    
    NodeType kind() final { return NodeType::ListDeclaration; }

    ListDeclarationNode(std::shared_ptr<Parsing::Node> var_type_p, Lexing::Token name_p, int dim_p, std::shared_ptr<Parsing::Node> initial_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> var_type_p, Lexing::Token name_p, int dim_p, std::shared_ptr<Parsing::Node> initial_p){
        return std::make_shared<ListDeclarationNode>(std::move(var_type_p), std::move(name_p), std::move(dim_p), std::move(initial_p));
    }
};
}
