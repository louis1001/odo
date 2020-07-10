
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ConstructorDeclNode final : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> params;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::ConstructorDecl; }

    ConstructorDeclNode(std::vector<std::shared_ptr<Parsing::Node>> params_p, std::shared_ptr<Parsing::Node> body_p);

    static std::shared_ptr<Node> create(std::vector<std::shared_ptr<Parsing::Node>> params_p, std::shared_ptr<Parsing::Node> body_p){
        return std::make_shared<ConstructorDeclNode>(std::move(params_p), std::move(body_p));
    }
};
}
