
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct IntNode : public Node {
    Lexing::Token t;
    
    NodeType kind() final { return NodeType::Int; }

    IntNode(Lexing::Token t_p);
};
}
