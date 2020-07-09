
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct WhileNode final : public Node {
    std::shared_ptr<Parsing::Node> cond;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::While; }

    WhileNode(std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> body_p);
};
}
