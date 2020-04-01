//
// Created by Luis Gonzalez on 3/23/20.
//

#include "utils.h"

bool contains_type(std::vector<TokenType> arr, TokenType t) {
    return std::find(arr.begin(), arr.end(), t) != arr.end();
}