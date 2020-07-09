
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ClassBodyNode final : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> statements;
    
    NodeType kind() final { return NodeType::ClassBody; }

    explicit ClassBodyNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p);
};
}
