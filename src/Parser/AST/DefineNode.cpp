
#include "Parser/AST/DefineNode.h"

namespace Odo::Parsing {

DefineNode::DefineNode(std::vector<std::pair<Lexing::Token, bool>> args_p, Lexing::Token retType_p, Lexing::Token name_p)
    : args(std::move(args_p))
    , retType(std::move(retType_p))
    , name(std::move(name_p)){}

}

