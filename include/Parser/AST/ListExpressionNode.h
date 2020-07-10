
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ListExpressionNode final : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> elements;
    
    NodeType kind() final { return NodeType::ListExpression; }

    explicit ListExpressionNode(std::vector<std::shared_ptr<Parsing::Node>> elements_p);

    static std::shared_ptr<Node> create(std::vector<std::shared_ptr<Parsing::Node>> elements_p){
        return std::make_shared<ListExpressionNode>(std::move(elements_p));
    }
};
}
