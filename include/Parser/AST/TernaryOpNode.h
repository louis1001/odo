
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct TernaryOpNode final : public Node {
    std::shared_ptr<Parsing::Node> cond;
    std::shared_ptr<Parsing::Node> trueb;
    std::shared_ptr<Parsing::Node> falseb;
    
    NodeType kind() final { return NodeType::TernaryOp; }

    TernaryOpNode(std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> trueb_p, std::shared_ptr<Parsing::Node> falseb_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> trueb_p, std::shared_ptr<Parsing::Node> falseb_p){
        return std::make_shared<TernaryOpNode>(std::move(cond_p), std::move(trueb_p), std::move(falseb_p));
    }
};
}
