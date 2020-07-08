
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ModuleNode final : public Node {
    Lexing::Token name;
    std::vector<std::shared_ptr<Parsing::Node>> statements;
    
    NodeType kind() final { return NodeType::Module; }

    ModuleNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> statements_p);
};
}
