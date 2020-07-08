
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct NullNode : public Node {

    NodeType kind() final { return NodeType::Null; }

    NullNode(){}
};
}
