//
// Created by Luis Gonzalez on 6/25/20.
//

#ifndef ODO_FRAME_H
#define ODO_FRAME_H
#include <string>
namespace Odo::Interpreting {
    struct Frame {
        std::string name;
        unsigned int line_number;
        unsigned int column_number;
    };
}

#endif //ODO_FRAME_H
