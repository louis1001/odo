
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct NoOpNode final : public Node {

    NodeType kind() final { return NodeType::NoOp; }

    NoOpNode(){}

    static std::shared_ptr<Node> create(){
        return std::make_shared<NoOpNode>();
    }
};
}
