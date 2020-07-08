
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct FuncDeclNode : public Node {
    Lexing::Token name;
    std::vector<std::shared_ptr<Parsing::Node>> params;
    Lexing::Token retType;
    std::shared_ptr<Parsing::Node> body;
    
    NodeType kind() final { return NodeType::FuncDecl; }

    FuncDeclNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p, Lexing::Token retType_p, std::shared_ptr<Parsing::Node> body_p);
};
}
