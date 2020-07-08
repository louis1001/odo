
#include "Parser/AST/BinOpNode.h"

namespace Odo::Parsing {

BinOpNode::BinOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> left_p, std::shared_ptr<Parsing::Node> right_p)
    : token(token_p)
    , left(left_p)
    , right(right_p){}

}

