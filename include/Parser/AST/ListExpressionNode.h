
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ListExpressionNode : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> elements;
    
    NodeType kind() final { return NodeType::ListExpression; }

    explicit ListExpressionNode(std::vector<std::shared_ptr<Parsing::Node>> elements_p);
};
}
