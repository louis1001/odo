//
// Created by louis1001 on 31/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_MATHMODULE_H
#define ODO_MATHMODULE_H
#include "Modules/NativeModule.h"
#include <numbers>

namespace Odo::Modules {
    class MathModule : public NativeModule{
    public:
        explicit MathModule(Interpreting::Interpreter& inter)
            : NativeModule(module_name(), inter)
        {
            add_literal("e", std::numbers::e);
            add_literal("pi", std::numbers::pi);

            // add_function("pow", arg_types, ret_type, [&](){});
        }
        std::string module_name() final { return "math"; }
    };
}

#endif //ODO_MATHMODULE_H
