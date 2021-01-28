//
// Created by louis1001 on 27/1/2021.
//

#ifndef ODO_TERMCOLORSMODULE_H
#define ODO_TERMCOLORSMODULE_H
#include "NativeModule.h"

namespace Odo::Modules {
    class TermColorsModule: NativeModule {
    public:
        explicit TermColorsModule(Interpreting::Interpreter& inter)
        : NativeModule(module_name(), inter)
        {

        }

        std::string module_name() final { return "color"; }
    };
}

#endif //ODO_TERMCOLORSMODULE_H
