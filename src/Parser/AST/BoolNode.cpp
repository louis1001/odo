
#include "Parser/AST/BoolNode.h"

namespace Odo::Parsing {

BoolNode::BoolNode(Lexing::Token token_p)
    : token(std::move(token_p)){}

}

