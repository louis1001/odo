
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct NullNode final : public Node {

    NodeType kind() final { return NodeType::Null; }

    NullNode(){}
};
}
