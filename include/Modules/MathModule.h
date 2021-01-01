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

            add_function(
                "pow",
                {{double_type, false}, {double_type, true}},
                double_type,
                [&](auto vals){
                    auto x = std::any_cast<double>(vals[0]);
                    double y = 2;
                    if (vals.size() == 2) {
                        y = std::any_cast<double>(vals[1]);
                    }
                    return pow(x, y);
                }
            );

            add_function("exp", {{double_type, false}}, double_type,
                [](auto vals){
                    return exp(std::any_cast<double>(vals[0]));
                }
            );
        }
        std::string module_name() final { return "math"; }
    };
}

#endif //ODO_MATHMODULE_H
