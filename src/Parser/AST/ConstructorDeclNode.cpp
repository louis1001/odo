
#include "Parser/AST/ConstructorDeclNode.h"

namespace Odo::Parsing {

ConstructorDeclNode::ConstructorDeclNode(std::vector<std::shared_ptr<Parsing::Node>> params_p, std::shared_ptr<Parsing::Node> body_p)
    : params(std::move(params_p))
    , body(std::move(body_p)){}

}

