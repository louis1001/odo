
#include "Parser/AST/FuncBodyNode.h"

namespace Odo::Parsing {

FuncBodyNode::FuncBodyNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p)
    : statements(statements_p){}

}

