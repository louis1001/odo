//
// Created by Luis Gonzalez on 4/27/20.
//

#ifndef ODO_IO_H
#define ODO_IO_H
#include <string>
#include <vector>

namespace Odo::io {
    std::string read_file(const std::string& path);
    std::string get_file_name(const std::string& path);

    std::string to_absolute_path(const std::string&);
    std::string get_cwd();
    bool path_exists(const std::string&);
    bool is_dir(const std::string&);
    bool is_file(const std::string&);
    std::vector<std::string> list_dir(const std::string&);
    void create_file(const std::string&);
    void write_to_file(const std::string &, const std::string &content);
    void append_to_file(const std::string &, const std::string &content);
}
#endif //ODO_IO_H
