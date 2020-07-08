
#include "Parser/AST/ClassNode.h"

namespace Odo::Parsing {

ClassNode::ClassNode(Lexing::Token name_p, Lexing::Token ty_p, std::shared_ptr<Parsing::Node> body_p)
    : name(name_p)
    , ty(ty_p)
    , body(body_p){}

}

