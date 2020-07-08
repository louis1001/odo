
#include "Parser/AST/BlockNode.h"

namespace Odo::Parsing {

BlockNode::BlockNode(std::vector<std::shared_ptr<Parsing::Node>> vector_p)
    : vector(vector_p){}

}

