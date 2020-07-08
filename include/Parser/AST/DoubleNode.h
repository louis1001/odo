
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct DoubleNode : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Double; }

    DoubleNode(Lexing::Token token_p);
};
}
