
#include "Parser/AST/LoopNode.h"

namespace Odo::Parsing {

LoopNode::LoopNode(std::shared_ptr<Parsing::Node> body_p)
    : body(body_p){}

}

