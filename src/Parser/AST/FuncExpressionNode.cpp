
#include "Parser/AST/FuncExpressionNode.h"

namespace Odo::Parsing {

FuncExpressionNode::FuncExpressionNode(std::vector<std::shared_ptr<Parsing::Node>> params_p, Lexing::Token retType_p, std::shared_ptr<Parsing::Node> body_p)
    : params(std::move(params_p))
    , retType(std::move(retType_p))
    , body(std::move(body_p)){}

}

