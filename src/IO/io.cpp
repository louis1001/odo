//
// Created by Luis Gonzalez on 4/27/20.
//

#include "IO/io.h"
#include <fstream>
#include <filesystem>
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

    std::string get_file_name(const std::string& path, bool ignore_extension) {
        std::size_t found = path.find_last_of(std::filesystem::path::preferred_separator);
        std::string result = path.substr(found+1);

        if (ignore_extension) {
            std::filesystem::path without_ext{result};
            without_ext.replace_extension();
            return without_ext.string();
        }

        return result;
    }

    std::string to_absolute_path(const std::string& rel_path) {
        return std::filesystem::absolute(rel_path).string();
    }

    std::string get_cwd() {
        return std::filesystem::current_path().string();
    }

    bool path_exists(const std::string& path) {
        return std::filesystem::exists(path);
    }

    bool is_dir(const std::string& path) {
        return path_exists(path) && std::filesystem::is_directory(path);
    }

    bool is_file(const std::string& path) {
        return path_exists(path) && std::filesystem::is_regular_file(path);
    }

    std::vector<std::string> list_dir(const std::string& path) {
        if (!path_exists(path)) {
            throw Exceptions::IOException(path);
        }

        std::vector<std::string> files;
        for (const auto& fn : std::filesystem::directory_iterator(path)) {
            files.push_back(fn.path().string());
        }

        return files;
    }

    void create_file(const std::string& path) {
        std::ofstream result(path);
    }

    void write_to_file(const std::string &path, const std::string &content) {
        std::ofstream result(path);
        if (result.is_open()) {
            result << content;
        } else {
            // IO error. Could not open or create file.
            throw Exceptions::IOException(path);
        }

        result.close();
    }

    void append_to_file(const std::string &path, const std::string &content) {
        std::ofstream result(path, std::ios_base::app);

        if (result.is_open()) {
            result << content;
        } else {
            // IO error. Could not open or create file.
            throw Exceptions::IOException(path);
        }

        result.close();
    }
}