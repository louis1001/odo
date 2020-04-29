//
// Created by Luis Gonzalez on 4/27/20.
//

#include "IO/io.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <Exceptions/exception.h>

namespace Odo::io {
    std::string read_file(const std::string& path) {
        std::filesystem::path a(path);
        std::ifstream argumentFile(path);
        if (argumentFile.fail()) {
            throw Exceptions::IOException(path);
        }

        std::stringstream sstr;
        sstr << argumentFile.rdbuf();
        std::string contents = sstr.str();
        argumentFile.close();

        return contents;
    }

}