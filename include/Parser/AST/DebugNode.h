
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct DebugNode final : public Node {

    NodeType kind() final { return NodeType::Debug; }

    DebugNode(){}
};
}
