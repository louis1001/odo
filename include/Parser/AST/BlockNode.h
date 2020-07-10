
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BlockNode final : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> statements;
    
    NodeType kind() final { return NodeType::Block; }

    explicit BlockNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p);

    static std::shared_ptr<Node> create(std::vector<std::shared_ptr<Parsing::Node>> statements_p){
        return std::make_shared<BlockNode>(std::move(statements_p));
    }
};
}
