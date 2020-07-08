
#include "Parser/AST/FuncCallNode.h"

namespace Odo::Parsing {

FuncCallNode::FuncCallNode(std::shared_ptr<Parsing::Node> expr_p, Lexing::Token fname_p, std::vector<std::shared_ptr<Parsing::Node>> args_p)
    : expr(expr_p)
    , fname(fname_p)
    , args(args_p){}

}

