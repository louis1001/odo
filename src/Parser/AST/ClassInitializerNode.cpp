
#include "Parser/AST/ClassInitializerNode.h"

namespace Odo::Parsing {

ClassInitializerNode::ClassInitializerNode(std::shared_ptr<Parsing::Node> cls_p, std::vector<std::shared_ptr<Parsing::Node>> params_p)
    : cls(std::move(cls_p))
    , params(std::move(params_p)){}

}

