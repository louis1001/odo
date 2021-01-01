//
// Created by louis1001 on 31/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_MATHMODULE_H
#define ODO_MATHMODULE_H
#include "Modules/NativeModule.h"
#include <numbers>
#include <cmath>

namespace Odo::Modules {
    class MathModule : public NativeModule{
    public:
        explicit MathModule(Interpreting::Interpreter& inter)
            : NativeModule(module_name(), inter)
        {
            add_literal(E_CONST, std::numbers::e);
            add_literal(PI_CONST, std::numbers::pi);

            add_function(FACTR_FN, {{int_type, false}}, int_type, [](auto vals){
                int bound = std::any_cast<int>(vals[0]);
                int result = 1;
                for(int i = 1; i <= bound; i++) {
                    result = result * i;
                }

                return result;
            });

            add_function(
                POW_FN,
                {{double_type, false}, {double_type, true}},
                double_type,
                [&](auto vals){
                    auto x = std::any_cast<double>(vals[0]);
                    double y = 2;
                    if (vals.size() == 2) {
                        y = std::any_cast<double>(vals[1]);
                    }
                    return std::pow(x, y);
                }
            );

            add_function(SQRT_FN, {{double_type, false}}, double_type, [](auto vals){
                return std::sqrt(std::any_cast<double>(vals[0]));
            });

            add_function(SIN_FN, {{double_type, false}}, double_type, [](auto vals){
                return std::sin(std::any_cast<double>(vals[0]));
            });

            add_function(COS_FN, {{double_type, false}}, double_type, [](auto vals){
                return std::cos(std::any_cast<double>(vals[0]));
            });

            add_function(FLOOR_FN, {{double_type, false}}, int_type, [](auto vals){
                return (int)std::floor(std::any_cast<double>(vals[0]));
            });

            add_function(TRUNC_FN, {{double_type, false}}, int_type, [](auto vals){
                return (int)std::trunc(std::any_cast<double>(vals[0]));
            });

            add_function(ROUND_FN, {{double_type, false}, {double_type, true}}, double_type, [](auto vals){
                auto val1 = std::any_cast<double>(vals[0]);
                double rounding = 0;
                if (vals.size() == 2) {
                    rounding = std::any_cast<double>(vals[1]);
                }

                int decimal = static_cast<int>(std::pow(10, rounding));
                double result = round(val1*decimal)/decimal;
                return result;
            });

            add_function(EXP_FN, {{double_type, false}}, double_type,
                [](auto vals){
                    return exp(std::any_cast<double>(vals[0]));
                }
            );
        }
        std::string module_name() final { return MATH_MD; }
    };
}

#endif //ODO_MATHMODULE_H
