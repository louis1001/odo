
#include "Parser/AST/ClassBodyNode.h"

namespace Odo::Parsing {

ClassBodyNode::ClassBodyNode(std::vector<std::shared_ptr<Parsing::Node>> statements_p)
    : statements(std::move(statements_p)){}

}

