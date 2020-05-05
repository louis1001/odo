//
// Created by Luis Gonzalez on 3/23/20.
//

#ifndef ODO_PORT_UTILS_H
#define ODO_PORT_UTILS_H
#include <vector>
#include "Interpreter/symbol.h"
#include "Lexer/token.hpp"

namespace Odo {
    bool contains_type(std::vector<Lexing::TokenType> arr, Lexing::TokenType t);
    bool ends_with(std::string const &, std::string const &);
    bool starts_with(std::string const &, std::string const &);
}
#endif //ODO_PORT_UTILS_H
