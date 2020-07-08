
#include "Parser/AST/ForEachNode.h"

namespace Odo::Parsing {

ForEachNode::ForEachNode(Lexing::Token var_p, std::shared_ptr<Parsing::Node> lst_p, std::shared_ptr<Parsing::Node> body_p, Lexing::Token rev_p)
    : var(std::move(var_p))
    , lst(std::move(lst_p))
    , body(std::move(body_p))
    , rev(std::move(rev_p)){}

}

