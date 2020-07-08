
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ConstructorCallNode : public Node {
    Lexing::Token t;
    
    NodeType kind() final { return NodeType::ConstructorCall; }

    ConstructorCallNode(Lexing::Token t_p);
};
}
