
#include "Parser/AST/WhileNode.h"

namespace Odo::Parsing {

WhileNode::WhileNode(std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> body_p)
    : cond(std::move(cond_p))
    , body(std::move(body_p)){}

}

