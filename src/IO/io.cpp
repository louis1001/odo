//
// Created by Luis Gonzalez on 4/27/20.
//

#include "IO/io.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace Odo::io {
    std::string read_file(const std::string& path) {
        std::filesystem::path a(path);
        std::ifstream argumentFile(path);
        if (argumentFile.fail()) {
            // Error Opening File
            std::filesystem::path abs(".");
            auto b_abs = abs.lexically_normal().string();
            auto b = a.lexically_normal().string();
            throw std::exception();
        }

        std::stringstream sstr;
        sstr << argumentFile.rdbuf();
        std::string contents = sstr.str();
        argumentFile.close();

        return contents;
    }

}