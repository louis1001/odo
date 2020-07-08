
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct MemberVarNode : public Node {
    std::shared_ptr<Parsing::Node> inst;
    Lexing::Token name;
    
    NodeType kind() final { return NodeType::MemberVar; }

    MemberVarNode(std::shared_ptr<Parsing::Node> inst_p, Lexing::Token name_p);
};
}