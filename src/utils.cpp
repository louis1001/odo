//
// Created by Luis Gonzalez on 3/23/20.
//

#include "utils.h"

namespace Odo {
    bool contains_type(std::vector<Lexing::TokenType> arr, Lexing::TokenType t) {
        return std::find(arr.begin(), arr.end(), t) != arr.end();
    }
}