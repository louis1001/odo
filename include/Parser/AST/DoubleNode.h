
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct DoubleNode : public Node {
    Lexing::Token t;
    
    NodeType kind() final { return NodeType::Double; }

    DoubleNode(Lexing::Token t_p);
};
}
