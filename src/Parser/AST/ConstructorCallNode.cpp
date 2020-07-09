
#include "Parser/AST/ConstructorCallNode.h"

namespace Odo::Parsing {

ConstructorCallNode::ConstructorCallNode(Lexing::Token t_p)
    : t(std::move(t_p)){}

}

