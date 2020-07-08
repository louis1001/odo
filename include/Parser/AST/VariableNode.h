
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct VariableNode : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Variable; }

    VariableNode(Lexing::Token token_p);
};
}
