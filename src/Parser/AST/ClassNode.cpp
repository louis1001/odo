
#include "Parser/AST/ClassNode.h"

namespace Odo::Parsing {

ClassNode::ClassNode(Lexing::Token name_p, Lexing::Token ty_p, std::shared_ptr<Parsing::Node> body_p)
    : name(std::move(name_p))
    , ty(std::move(ty_p))
    , body(std::move(body_p)){}

}

