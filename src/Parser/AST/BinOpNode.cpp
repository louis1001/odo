
#include "Parser/AST/BinOpNode.h"

namespace Odo::Parsing {

BinOpNode::BinOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> ast_p, std::shared_ptr<Parsing::Node> ast1_p)
    : token(token_p)
    , ast(ast_p)
    , ast1(ast1_p){}

}

