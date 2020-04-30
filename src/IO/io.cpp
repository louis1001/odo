//
// Created by Luis Gonzalez on 4/27/20.
//

#include "IO/io.h"
#include <fstream>
#include <Exceptions/exception.h>

namespace Odo::io {
    std::string read_file(const std::string& path) {
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

    std::string get_file_name(const std::string& path) {
        std::size_t found = path.find_last_of("/\\");
        std::string result = path.substr(found+1);

        return result;
    }
}