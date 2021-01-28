
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FuncExpressionNode final : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> params;
    std::shared_ptr<Parsing::Node> retType;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::FuncExpression; }

    FuncExpressionNode(std::vector<std::shared_ptr<Parsing::Node>> params_p, std::shared_ptr<Parsing::Node> retType_p, std::shared_ptr<Parsing::Node> body_p);

    static std::shared_ptr<Node> create(std::vector<std::shared_ptr<Parsing::Node>> params_p, std::shared_ptr<Parsing::Node> retType_p, std::shared_ptr<Parsing::Node> body_p){
        return std::make_shared<FuncExpressionNode>(std::move(params_p), std::move(retType_p), std::move(body_p));
    }
};
}
