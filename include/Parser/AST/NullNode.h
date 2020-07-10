
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct NullNode final : public Node {

    NodeType kind() final { return NodeType::Null; }

    NullNode(){}

    static std::shared_ptr<Node> create(){
        return std::make_shared<NullNode>();
    }
};
}
