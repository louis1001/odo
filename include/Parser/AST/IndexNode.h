
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct IndexNode : public Node {
    std::shared_ptr<Parsing::Node> val;
    std::shared_ptr<Parsing::Node> expr;
    
    NodeType kind() final { return NodeType::Index; }

    IndexNode(std::shared_ptr<Parsing::Node> val_p, std::shared_ptr<Parsing::Node> expr_p);
};
}
