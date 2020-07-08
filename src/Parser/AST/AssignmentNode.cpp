
#include "Parser/AST/AssignmentNode.h"

namespace Odo::Parsing {

AssignmentNode::AssignmentNode(std::shared_ptr<Parsing::Node> expr_p, std::shared_ptr<Parsing::Node> val_p)
    : expr(expr_p)
    , val(val_p){}

}

