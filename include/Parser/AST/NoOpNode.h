
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct NoOpNode : public Node {

    NodeType kind() final { return NodeType::NoOp; }

    NoOpNode(){}
};
}
