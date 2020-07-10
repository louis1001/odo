
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FuncDeclNode final : public Node {
    Lexing::Token name;
    std::vector<std::shared_ptr<Parsing::Node>> params;
    Lexing::Token retType;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::FuncDecl; }

    FuncDeclNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p, Lexing::Token retType_p, std::shared_ptr<Parsing::Node> body_p);

    static std::shared_ptr<Node> create(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p, Lexing::Token retType_p, std::shared_ptr<Parsing::Node> body_p){
        return std::make_shared<FuncDeclNode>(std::move(name_p), std::move(params_p), std::move(retType_p), std::move(body_p));
    }
};
}
