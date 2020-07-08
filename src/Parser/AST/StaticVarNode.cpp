
#include "Parser/AST/StaticVarNode.h"

namespace Odo::Parsing {

StaticVarNode::StaticVarNode(std::shared_ptr<Parsing::Node> inst_p, Lexing::Token name_p)
    : inst(std::move(inst_p))
    , name(std::move(name_p)){}

}

