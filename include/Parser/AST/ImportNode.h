
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ImportNode : public Node {
    Lexing::Token path;
    Lexing::Token name;
    
    NodeType kind() final { return NodeType::Import; }

    ImportNode(Lexing::Token path_p, Lexing::Token name_p);
};
}
