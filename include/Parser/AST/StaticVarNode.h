
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct StaticVarNode final : public Node {
    std::shared_ptr<Parsing::Node> inst;
    Lexing::Token name;
    
    NodeType kind() final { return NodeType::StaticVar; }

    StaticVarNode(std::shared_ptr<Parsing::Node> inst_p, Lexing::Token name_p);
};
}
