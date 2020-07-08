
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct VarDeclarationNode : public Node {
    Lexing::Token var_type;
    Lexing::Token name;
    std::shared_ptr<Parsing::Node> initial;
    
    NodeType kind() final { return NodeType::VarDeclaration; }

    VarDeclarationNode(Lexing::Token var_type_p, Lexing::Token name_p, std::shared_ptr<Parsing::Node> initial_p);
};
}
