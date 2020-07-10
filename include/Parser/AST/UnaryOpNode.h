
#pragma once

#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct UnaryOpNode final : public Node {
    Lexing::Token token;
    std::shared_ptr<Parsing::Node> ast;
    
    NodeType kind() final { return NodeType::UnaryOp; }

    UnaryOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> ast_p);

    static std::shared_ptr<Node> create(Lexing::Token token_p, std::shared_ptr<Parsing::Node> ast_p){
        return std::make_shared<UnaryOpNode>(std::move(token_p), std::move(ast_p));
    }
};
}
