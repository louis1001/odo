
#include "Parser/AST/VariableNode.h"

namespace Odo::Parsing {

VariableNode::VariableNode(Lexing::Token token_p)
    : token(std::move(token_p)){}

}

