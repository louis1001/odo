
#include "Parser/AST/StaticStatementNode.h"

namespace Odo::Parsing {

StaticStatementNode::StaticStatementNode(std::shared_ptr<Node> statement_p)
    : statement(std::move(statement_p)){}

}

