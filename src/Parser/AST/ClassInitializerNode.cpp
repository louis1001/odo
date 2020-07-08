
#include "Parser/AST/ClassInitializerNode.h"

namespace Odo::Parsing {

ClassInitializerNode::ClassInitializerNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p)
    : name(name_p)
    , params(params_p){}

}

