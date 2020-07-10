
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ForNode final : public Node {
    std::shared_ptr<Parsing::Node> ini;
    std::shared_ptr<Parsing::Node> cond;
    std::shared_ptr<Parsing::Node> incr;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::For; }

    ForNode(std::shared_ptr<Parsing::Node> ini_p, std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> incr_p, std::shared_ptr<Parsing::Node> body_p);

    static std::shared_ptr<Node> create(std::shared_ptr<Parsing::Node> ini_p, std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> incr_p, std::shared_ptr<Parsing::Node> body_p){
        return std::make_shared<ForNode>(std::move(ini_p), std::move(cond_p), std::move(incr_p), std::move(body_p));
    }
};
}
