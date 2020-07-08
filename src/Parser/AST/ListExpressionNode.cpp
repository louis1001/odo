
#include "Parser/AST/ListExpressionNode.h"

namespace Odo::Parsing {

ListExpressionNode::ListExpressionNode(std::vector<std::shared_ptr<Parsing::Node>> elements_p)
    : elements(elements_p){}

}

