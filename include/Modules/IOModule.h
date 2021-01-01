//
// Created by louis1001 on 31/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_IOMODULE_H
#define ODO_IOMODULE_H
#include "Interpreter/Interpreter.h"
#include "NativeModule.h"
#include "IO/io.h"

namespace Odo::Modules {
    class IOModule : public NativeModule {
    public:
        explicit IOModule(Interpreting::Interpreter& inter)
        : NativeModule(module_name(), inter)
        {
            add_function(WRITE_TO_FILE_FN, {{string_type, false}, {string_type, false}}, nullptr,
                [](auto vals){
                    auto path = std::any_cast<std::string>(vals[0]);
                    auto content = std::any_cast<std::string>(vals[1]);

                    io::write_to_file(path, content);
                    return nullptr;
                }
            );
        }
        std::string module_name() final { return IO_MD; }
    };
}

#endif //ODO_IOMODULE_H
