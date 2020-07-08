
#include "Parser/AST/FoRangeNode.h"

namespace Odo::Parsing {

FoRangeNode::FoRangeNode(Lexing::Token var_p, std::shared_ptr<Parsing::Node> first_p, std::shared_ptr<Parsing::Node> second_p, std::shared_ptr<Parsing::Node> body_p, Lexing::Token rev_p)
    : var(var_p)
    , first(first_p)
    , second(second_p)
    , body(body_p)
    , rev(rev_p){}

}

