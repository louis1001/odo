
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct ClassInitializerNode : public Node {
    Lexing::Token name;
    std::vector<std::shared_ptr<Parsing::Node>> params;
    
    NodeType kind() final { return NodeType::ClassInitializer; }

    ClassInitializerNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> params_p);
};
}
