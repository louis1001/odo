
#include "Parser/AST/IfNode.h"

namespace Odo::Parsing {

IfNode::IfNode(std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> trueb_p, std::shared_ptr<Parsing::Node> falseb_p)
    : cond(cond_p)
    , trueb(trueb_p)
    , falseb(falseb_p){}

}

