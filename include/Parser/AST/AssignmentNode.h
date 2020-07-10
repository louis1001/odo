
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct AssignmentNode final : public Node {
    std::shared_ptr<Parsing::Node> expr;
    std::shared_ptr<Parsing::Node> val;
    
    NodeType kind() final { return NodeType::Assignment; }

    AssignmentNode(std::shared_ptr<Parsing::Node> expr_p, std::shared_ptr<Parsing::Node> val_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> expr_p, std::shared_ptr<Parsing::Node> val_p){
        return std::make_shared<AssignmentNode>(std::move(expr_p), std::move(val_p));
    }
};
}
