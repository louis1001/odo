
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ReturnNode : public Node {
    std::shared_ptr<Parsing::Node> val;
    
    NodeType kind() final { return NodeType::Return; }

    explicit ReturnNode(std::shared_ptr<Parsing::Node> val_p);
};
}
