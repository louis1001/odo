
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BlockNode : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> vector;
    
    NodeType kind() final { return NodeType::Block; }

    BlockNode(std::vector<std::shared_ptr<Parsing::Node>> vector_p);
};
}
