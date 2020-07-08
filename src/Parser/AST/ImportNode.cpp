
#include "Parser/AST/ImportNode.h"

namespace Odo::Parsing {

ImportNode::ImportNode(Lexing::Token path_p, Lexing::Token name_p)
    : path(path_p)
    , name(name_p){}

}

