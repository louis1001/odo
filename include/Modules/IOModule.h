//
// Created by louis1001 on 31/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_IOMODULE_H
#define ODO_IOMODULE_H
#include "Interpreter/Interpreter.h"
#include "NativeModule.h"

namespace Odo::Modules {
    class IOModule : public NativeModule {
    public:
        explicit IOModule(Interpreting::Interpreter& inter)
        : NativeModule(module_name(), inter)
        {
            add_literal("hostname", std::string("louis1001"));
            add_literal("working", true);
        }
        std::string module_name() final { return "io"; }
    };
}

#endif //ODO_IOMODULE_H
