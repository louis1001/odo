
#include "Parser/AST/StrNode.h"

namespace Odo::Parsing {

StrNode::StrNode(Lexing::Token token_p)
    : token(std::move(token_p)){}

}

