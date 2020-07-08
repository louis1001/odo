
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct InstanceBodyNode : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> statements;
    
    NodeType kind() final { return NodeType::InstanceBody; }

    explicit InstanceBodyNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p);
};
}
