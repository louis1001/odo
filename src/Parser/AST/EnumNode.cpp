
#include "Parser/AST/EnumNode.h"

namespace Odo::Parsing {

EnumNode::EnumNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> variants_p)
    : name(name_p)
    , variants(variants_p){}

}

