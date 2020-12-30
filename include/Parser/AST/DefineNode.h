
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct DefineNode final : public Node {
    std::vector<std::pair<Lexing::Token, bool>> args;
    Lexing::Token retType;
    Lexing::Token name;
    
    NodeType kind() final { return NodeType::Define; }

    DefineNode(std::vector<std::pair<Lexing::Token, bool>> args_p, Lexing::Token retType_p, Lexing::Token name_p);

    static std::shared_ptr<Node> create(std::vector<std::pair<Lexing::Token, bool>> args_p, Lexing::Token retType_p, Lexing::Token name_p){
        return std::make_shared<DefineNode>(std::move(args_p), std::move(retType_p), std::move(name_p));
    }
};
}
