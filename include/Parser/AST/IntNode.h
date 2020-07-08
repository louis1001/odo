
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct IntNode : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Int; }

    IntNode(Lexing::Token token_p);
};
}
