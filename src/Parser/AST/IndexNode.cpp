
#include "Parser/AST/IndexNode.h"

namespace Odo::Parsing {

IndexNode::IndexNode(std::shared_ptr<Parsing::Node> val_p, std::shared_ptr<Parsing::Node> expr_p)
    : val(std::move(val_p))
    , expr(std::move(expr_p)){}

}

