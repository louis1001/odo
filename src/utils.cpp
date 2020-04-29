//
// Created by Luis Gonzalez on 3/23/20.
//

#include "utils.h"

namespace Odo {
    bool contains_type(std::vector<Lexing::TokenType> arr, Lexing::TokenType t) {
        return std::find(arr.begin(), arr.end(), t) != arr.end();
    }

    bool ends_with(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }
}