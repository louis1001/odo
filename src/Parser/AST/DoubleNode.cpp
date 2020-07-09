
#include "Parser/AST/DoubleNode.h"

namespace Odo::Parsing {

DoubleNode::DoubleNode(Lexing::Token token_p)
    : token(std::move(token_p)){}

}

