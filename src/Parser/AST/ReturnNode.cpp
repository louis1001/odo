
#include "Parser/AST/ReturnNode.h"

namespace Odo::Parsing {

ReturnNode::ReturnNode(std::shared_ptr<Parsing::Node> val_p)
    : val(std::move(val_p)){}

}

