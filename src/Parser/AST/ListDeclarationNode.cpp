
#include "Parser/AST/ListDeclarationNode.h"

namespace Odo::Parsing {

ListDeclarationNode::ListDeclarationNode(Lexing::Token var_type_p, Lexing::Token name_p, std::shared_ptr<Parsing::Node> initial_p)
    : var_type(std::move(var_type_p))
    , name(std::move(name_p))
    , initial(std::move(initial_p)){}

}

