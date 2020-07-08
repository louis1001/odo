
#include "Parser/AST/TernaryOpNode.h"

namespace Odo::Parsing {

TernaryOpNode::TernaryOpNode(std::shared_ptr<Parsing::Node> cond_p, std::shared_ptr<Parsing::Node> trueb_p, std::shared_ptr<Parsing::Node> falseb_p)
    : cond(cond_p)
    , trueb(trueb_p)
    , falseb(falseb_p){}

}

