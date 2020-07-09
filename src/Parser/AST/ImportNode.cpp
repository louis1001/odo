
#include "Parser/AST/ImportNode.h"

namespace Odo::Parsing {

ImportNode::ImportNode(Lexing::Token path_p, Lexing::Token name_p)
    : path(std::move(path_p))
    , name(std::move(name_p)){}

}

