
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ClassBodyNode : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> statements;
    
    NodeType kind() final { return NodeType::ClassBody; }

    ClassBodyNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p);
};
}
