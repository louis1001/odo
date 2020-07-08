
#include "Parser/AST/ForEachNode.h"

namespace Odo::Parsing {

ForEachNode::ForEachNode(Lexing::Token v_p, std::shared_ptr<Parsing::Node> lst_p, std::shared_ptr<Parsing::Node> body_p, Lexing::Token rev_p)
    : v(v_p)
    , lst(lst_p)
    , body(body_p)
    , rev(rev_p){}

}

