
#include "Parser/AST/ConstructorDeclNode.h"

namespace Odo::Parsing {

ConstructorDeclNode::ConstructorDeclNode(std::vector<std::shared_ptr<Parsing::Node>> params_p, std::shared_ptr<Parsing::Node> body_p)
    : params(params_p)
    , body(body_p){}

}

