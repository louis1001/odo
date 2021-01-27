
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ClassInitializerNode final : public Node {
    std::shared_ptr<Parsing::Node> cls;
    std::vector<std::shared_ptr<Parsing::Node>> params;
    
    NodeType kind() final { return NodeType::ClassInitializer; }

    ClassInitializerNode(std::shared_ptr<Parsing::Node> cls_p, std::vector<std::shared_ptr<Parsing::Node>> params_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> cls_p, std::vector<std::shared_ptr<Parsing::Node>> params_p){
        return std::make_shared<ClassInitializerNode>(std::move(cls_p), std::move(params_p));
    }
};
}
