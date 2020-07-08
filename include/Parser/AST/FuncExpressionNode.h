
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FuncExpressionNode final : public Node {
    std::vector<std::shared_ptr<Parsing::Node>> params;
    Lexing::Token retType;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::FuncExpression; }

    FuncExpressionNode(std::vector<std::shared_ptr<Parsing::Node>> params_p, Lexing::Token retType_p, std::shared_ptr<Parsing::Node> body_p);
};
}
