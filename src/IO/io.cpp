//
// Created by Luis Gonzalez on 4/27/20.
//

#include "IO/io.h"
#include <fstream>
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