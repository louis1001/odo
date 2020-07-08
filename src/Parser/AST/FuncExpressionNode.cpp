
#include "Parser/AST/FuncExpressionNode.h"

namespace Odo::Parsing {

FuncExpressionNode::FuncExpressionNode(std::vector<std::shared_ptr<Parsing::Node>> params_p, Lexing::Token retType_p, std::shared_ptr<Parsing::Node> body_p)
    : params(params_p)
    , retType(retType_p)
    , body(body_p){}

}

