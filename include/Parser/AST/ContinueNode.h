
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ContinueNode final : public Node {

    NodeType kind() final { return NodeType::Continue; }

    ContinueNode(){}
};
}
