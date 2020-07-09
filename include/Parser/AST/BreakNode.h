
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BreakNode final : public Node {

    NodeType kind() final { return NodeType::Break; }

    BreakNode(){}
};
}
