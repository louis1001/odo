
#include "Parser/AST/EnumNode.h"

namespace Odo::Parsing {

EnumNode::EnumNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> variants_p)
    : name(std::move(name_p))
    , variants(std::move(variants_p)){}

}

