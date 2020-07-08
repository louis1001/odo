
#include "Parser/AST/StaticVarNode.h"

namespace Odo::Parsing {

StaticVarNode::StaticVarNode(std::shared_ptr<Parsing::Node> inst_p, Lexing::Token name_p)
    : inst(inst_p)
    , name(name_p){}

}

