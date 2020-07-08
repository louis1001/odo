
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BreakNode : public Node {

    NodeType kind() final { return NodeType::Break; }

    BreakNode(){}
};
}
