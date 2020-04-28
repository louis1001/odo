//
// Created by Luis Gonzalez on 3/23/20.
//

#include "utils.h"
#include <filesystem>

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

    bool has_extension(const std::string& path) {
        std::filesystem::path p(path);
        return p.has_extension();
    }

    std::string get_file_name(const std::string& path) {
        std::filesystem::path p(path);
        std::string result;
        std::string ext = p.extension();
        auto fn = p.filename().string();
        for (auto i = 0; i < fn.length()-ext.length(); i++) {
            result += fn[i];
        }

        return result;
    }
}