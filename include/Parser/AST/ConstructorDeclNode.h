
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ConstructorDeclNode : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> params;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::ConstructorDecl; }

    ConstructorDeclNode(std::vector<std::shared_ptr<Parsing::Node>> params_p, std::shared_ptr<Parsing::Node> body_p);
};
}
