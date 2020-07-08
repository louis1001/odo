
#include "Parser/AST/FuncDeclNode.h"

namespace Odo::Parsing {

FuncDeclNode::FuncDeclNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p, Lexing::Token retType_p, std::shared_ptr<Parsing::Node> body_p)
    : name(name_p)
    , params(params_p)
    , retType(retType_p)
    , body(body_p){}

}

