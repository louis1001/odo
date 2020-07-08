
#include "Parser/AST/IntNode.h"

namespace Odo::Parsing {

IntNode::IntNode(Lexing::Token token_p)
    : token(std::move(token_p)){}

}

