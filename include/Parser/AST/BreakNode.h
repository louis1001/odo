
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BreakNode final : public Node {

    NodeType kind() final { return NodeType::Break; }

    BreakNode(){}

    static std::shared_ptr<Node> create(){
        return std::make_shared<BreakNode>();
    }
};
}
