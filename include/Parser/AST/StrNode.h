
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct StrNode : public Node {
    Lexing::Token t;
    
    NodeType kind() final { return NodeType::Str; }

    StrNode(Lexing::Token t_p);
};
}
