
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FuncBodyNode : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> statements;
    
    NodeType kind() final { return NodeType::FuncBody; }

    explicit FuncBodyNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p);
};
}
