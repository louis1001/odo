#include "Lexer/token.hpp"

namespace Odo::Lexing {
    Token::Token(TokenType tp_, std::string val_) : tp(tp_), value(val_) {}
}
