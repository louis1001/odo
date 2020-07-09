
#include "Parser/AST/IfNode.h"

namespace Odo::Parsing {

IfNode::IfNode(std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> trueb_p, std::shared_ptr<Parsing::Node> falseb_p)
    : cond(std::move(cond_p))
    , trueb(std::move(trueb_p))
    , falseb(std::move(falseb_p)){}

}

