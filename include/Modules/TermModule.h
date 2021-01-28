//
// Created by louis1001 on 27/1/2021.
//

#ifndef ODO_TERMMODULE_H
#define ODO_TERMMODULE_H
#include "NativeModule.h"
#include "TermColorsModule.h"

namespace Odo::Modules {
    class TermModule: NativeModule {
    public:
        explicit TermModule(Interpreting::Interpreter& inter)
        : NativeModule(module_name(), inter)
        {
            add_module<TermColorsModule>(inter);
        }

        std::string module_name() final { return "term"; }
    };
}

#endif //ODO_TERMMODULE_H
