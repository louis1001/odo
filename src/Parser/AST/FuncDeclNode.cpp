
#include "Parser/AST/FuncDeclNode.h"

namespace Odo::Parsing {

FuncDeclNode::FuncDeclNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p, std::shared_ptr<Parsing::Node> retType_p, std::shared_ptr<Parsing::Node> body_p)
    : name(std::move(name_p))
    , params(std::move(params_p))
    , retType(std::move(retType_p))
    , body(std::move(body_p)){}

}

