
#include "Parser/AST/BinOpNode.h"

namespace Odo::Parsing {

BinOpNode::BinOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> left_p, std::shared_ptr<Parsing::Node> right_p)
    : token(std::move(token_p))
    , left(std::move(left_p))
    , right(std::move(right_p)){}

}

