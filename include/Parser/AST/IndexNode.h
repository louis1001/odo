
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct IndexNode final : public Node {
    std::shared_ptr<Parsing::Node> val;
    std::shared_ptr<Parsing::Node> expr;
    
    NodeType kind() final { return NodeType::Index; }

    IndexNode(std::shared_ptr<Parsing::Node> val_p, std::shared_ptr<Parsing::Node> expr_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> val_p, std::shared_ptr<Parsing::Node> expr_p){
        return std::make_shared<IndexNode>(std::move(val_p), std::move(expr_p));
    }
};
}
