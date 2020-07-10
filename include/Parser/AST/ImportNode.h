
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ImportNode final : public Node {
    Lexing::Token path;
    Lexing::Token name;
    
    NodeType kind() final { return NodeType::Import; }

    ImportNode(Lexing::Token path_p, Lexing::Token name_p);

    static std::shared_ptr<Node> create(Lexing::Token path_p, Lexing::Token name_p){
        return std::make_shared<ImportNode>(std::move(path_p), std::move(name_p));
    }
};
}
