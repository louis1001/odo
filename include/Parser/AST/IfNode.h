
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct IfNode : public Node {
    std::shared_ptr<Parsing::Node> cond;
    std::shared_ptr<Parsing::Node> trueb;
    std::shared_ptr<Parsing::Node> falseb;
    
    NodeType kind() final { return NodeType::If; }

    IfNode(std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> trueb_p, std::shared_ptr<Parsing::Node> falseb_p);
};
}
