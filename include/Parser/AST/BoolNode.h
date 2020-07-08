
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct BoolNode : public Node {
    Lexing::Token t;
    
    NodeType kind() final { return NodeType::Bool; }

    BoolNode(Lexing::Token t_p);
};
}
