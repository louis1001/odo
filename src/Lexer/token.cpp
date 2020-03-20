#include "Lexer/token.hpp"

Token::Token(TokenType tp_, std::string val_): tp(tp_), value(val_) {

}

bool contains_type(std::vector<TokenType> arr, TokenType t) {
    return std::find(arr.begin(), arr.end(), t) != arr.end();
}
