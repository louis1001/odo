
#include "Parser/AST/MemberVarNode.h"

namespace Odo::Parsing {

MemberVarNode::MemberVarNode(std::shared_ptr<Parsing::Node> inst_p, Lexing::Token name_p)
    : inst(std::move(inst_p))
    , name(std::move(name_p)){}

}

