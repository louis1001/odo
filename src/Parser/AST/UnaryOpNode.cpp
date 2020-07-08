
#include "Parser/AST/UnaryOpNode.h"

namespace Odo::Parsing {

UnaryOpNode::UnaryOpNode(Lexing::Token token_p, std::shared_ptr<Parsing::Node> ast_p)
    : token(token_p)
    , ast(ast_p){}

}

