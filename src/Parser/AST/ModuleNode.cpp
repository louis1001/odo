
#include "Parser/AST/ModuleNode.h"

namespace Odo::Parsing {

ModuleNode::ModuleNode(Lexing::Token name_p, std::vector<std::shared_ptr<Parsing::Node>> statements_p)
    : name(std::move(name_p))
    , statements(std::move(statements_p)){}

}

