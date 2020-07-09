
#include "Parser/AST/ForNode.h"

namespace Odo::Parsing {

ForNode::ForNode(std::shared_ptr<Parsing::Node> ini_p, std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> incr_p, std::shared_ptr<Parsing::Node> body_p)
    : ini(std::move(ini_p))
    , cond(std::move(cond_p))
    , incr(std::move(incr_p))
    , body(std::move(body_p)){}

}

