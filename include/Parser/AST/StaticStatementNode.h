
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct StaticStatementNode : public Node {
    std::shared_ptr<Node> statement;
    
    NodeType kind() final { return NodeType::StaticStatement; }

    explicit StaticStatementNode(std::shared_ptr<Node> statement_p);
};
}
