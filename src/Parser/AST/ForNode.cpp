
#include "Parser/AST/ForNode.h"

namespace Odo::Parsing {

ForNode::ForNode(std::shared_ptr<Parsing::Node> ini_p, std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> incr_p, std::shared_ptr<Parsing::Node> body_p)
    : ini(ini_p)
    , cond(cond_p)
    , incr(incr_p)
    , body(body_p){}

}

