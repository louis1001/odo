
#include "Parser/AST/BlockNode.h"

namespace Odo::Parsing {

BlockNode::BlockNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p)
    : statements(statements_p){}

}

