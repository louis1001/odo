
#include "Parser/AST/AssignmentNode.h"

namespace Odo::Parsing {

AssignmentNode::AssignmentNode(std::shared_ptr<Parsing::Node> expr_p, std::shared_ptr<Parsing::Node> val_p)
    : expr(std::move(expr_p))
    , val(std::move(val_p)){}

}

