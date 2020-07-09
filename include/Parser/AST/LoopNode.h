
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct LoopNode final : public Node {
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::Loop; }

    explicit LoopNode(std::shared_ptr<Parsing::Node> body_p);
};
}
