
#include "Parser/AST/ListDeclarationNode.h"

namespace Odo::Parsing {

ListDeclarationNode::ListDeclarationNode(Lexing::Token var_type_p, Lexing::Token name_p, std::shared_ptr<Parsing::Node> initial_p)
    : var_type(var_type_p)
    , name(name_p)
    , initial(initial_p){}

}

