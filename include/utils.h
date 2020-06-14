//
// Created by Luis Gonzalez on 3/23/20.
//

#ifndef ODO_PORT_UTILS_H
#define ODO_PORT_UTILS_H
#include <vector>
#include <random>
#include "Lexer/token.hpp"

namespace Odo {
    extern std::default_random_engine generator;

    bool contains_type(std::vector<Lexing::TokenType> arr, Lexing::TokenType t);
    bool ends_with(std::string const &, std::string const &);
    bool starts_with(std::string const &, std::string const &);
    int rand_int(int min, int max);
    double rand_double(double min, double max);
    double rand_double(double max=1);
}
#endif //ODO_PORT_UTILS_H
