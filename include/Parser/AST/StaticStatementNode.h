
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct StaticStatementNode final : public Node {
    std::shared_ptr<Node> statement;
    
    NodeType kind() final { return NodeType::StaticStatement; }

    explicit StaticStatementNode(std::shared_ptr<Node> statement_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Node> statement_p){
        return std::make_shared<StaticStatementNode>(std::move(statement_p));
    }
};
}
