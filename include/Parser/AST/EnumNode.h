
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct EnumNode : public Node {
    Lexing::Token name;
    std::vector<std::shared_ptr<Parsing::Node>> variants;
    
    NodeType kind() final { return NodeType::Enum; }

    EnumNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> variants_p);
};
}
