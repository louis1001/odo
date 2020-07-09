
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ForEachNode final : public Node {
    Lexing::Token var;
    std::shared_ptr<Parsing::Node> lst;
    std::shared_ptr<Parsing::Node> body;
    Lexing::Token rev;
    
    NodeType kind() final { return NodeType::ForEach; }

    ForEachNode(Lexing::Token var_p, std::shared_ptr<Parsing::Node> lst_p, std::shared_ptr<Parsing::Node> body_p, Lexing::Token rev_p);
};
}
