
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BlockNode : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> statements;
    
    NodeType kind() final { return NodeType::Block; }

    explicit BlockNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p);
};
}
