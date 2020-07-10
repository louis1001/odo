
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ClassInitializerNode final : public Node {
    Lexing::Token name;
    std::vector<std::shared_ptr<Parsing::Node>> params;
    
    NodeType kind() final { return NodeType::ClassInitializer; }

    ClassInitializerNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p);

    static std::shared_ptr<Node> create(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p){
        return std::make_shared<ClassInitializerNode>(std::move(name_p), std::move(params_p));
    }
};
}
