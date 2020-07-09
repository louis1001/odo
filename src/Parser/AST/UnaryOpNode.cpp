
#include "Parser/AST/UnaryOpNode.h"

namespace Odo::Parsing {

UnaryOpNode::UnaryOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> ast_p)
    : token(std::move(token_p))
    , ast(std::move(ast_p)){}

}

