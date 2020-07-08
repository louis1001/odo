
#include "Parser/AST/InstanceBodyNode.h"

namespace Odo::Parsing {

InstanceBodyNode::InstanceBodyNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p)
    : statements(statements_p){}

}

