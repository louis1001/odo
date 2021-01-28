
#include "Parser/AST/ListDeclarationNode.h"

namespace Odo::Parsing {

ListDeclarationNode::ListDeclarationNode(std::shared_ptr<Parsing::Node> var_type_p, Lexing::Token name_p, int dim_p, std::shared_ptr<Parsing::Node> initial_p)
    : var_type(std::move(var_type_p))
    , name(std::move(name_p))
    , dim(std::move(dim_p))
    , initial(std::move(initial_p)){}

}

