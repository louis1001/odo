
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct StrNode : public Node {
    Lexing::Token token;
    
    NodeType kind() final { return NodeType::Str; }

    StrNode(Lexing::Token token_p);
};
}
