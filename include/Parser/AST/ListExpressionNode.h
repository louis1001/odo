
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ListExpressionNode : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> elements;
    
    NodeType kind() final { return NodeType::ListExpression; }

    ListExpressionNode(std::vector<std::shared_ptr<Parsing::Node>> elements_p);
};
}
