
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FuncBodyNode final : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> statements;
    
    NodeType kind() final { return NodeType::FuncBody; }

    explicit FuncBodyNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p);

    static std::shared_ptr<Node> create(std::vector<std::shared_ptr<Parsing::Node>> statements_p){
        return std::make_shared<FuncBodyNode>(std::move(statements_p));
    }
};
}
