
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ReturnNode final : public Node {
    std::shared_ptr<Parsing::Node> val;
    
    NodeType kind() final { return NodeType::Return; }

    explicit ReturnNode(std::shared_ptr<Parsing::Node> val_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> val_p){
        return std::make_shared<ReturnNode>(std::move(val_p));
    }
};
}
