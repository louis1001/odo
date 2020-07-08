
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ContinueNode : public Node {

    NodeType kind() final { return NodeType::Continue; }

    ContinueNode(){}
};
}
