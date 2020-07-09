
#include "Parser/AST/FoRangeNode.h"

namespace Odo::Parsing {

FoRangeNode::FoRangeNode(Lexing::Token var_p, std::shared_ptr<Parsing::Node> first_p, std::shared_ptr<Parsing::Node> second_p, std::shared_ptr<Parsing::Node> body_p, Lexing::Token rev_p)
    : var(std::move(var_p))
    , first(std::move(first_p))
    , second(std::move(second_p))
    , body(std::move(body_p))
    , rev(std::move(rev_p)){}

}

