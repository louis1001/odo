//
// Created by louis1001 on 31/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_NATIVEMODULE_H
#define ODO_NATIVEMODULE_H
#include "Interpreter/value.h"

namespace Odo::Semantics {
    class SemanticAnalyzer;
}

namespace Odo::Modules {
    class NativeModule: public Interpreting::ModuleValue {
    protected:
        // Change this to be dynamic. Primitive types could change later.
        Interpreting::Symbol* int_type;
        Interpreting::Symbol* double_type;
        Interpreting::Symbol* bool_type;
        Interpreting::Symbol* string_type;

        std::weak_ptr<Semantics::SemanticAnalyzer> analyzer;

        void add_literal(const std::string&, const std::string&);
        void add_literal(const std::string&, int);
        void add_literal(const std::string&, double);
        void add_literal(const std::string&, bool);

        void add_function(const std::string &name,
                          const std::vector<std::pair<Interpreting::Symbol *, bool>> &arg_types,
                          Interpreting::Symbol *ret, std::function<std::any(std::vector<std::any>)> callback);

    public:
        explicit NativeModule(const std::string&, Interpreting::Interpreter& inter);
        virtual std::string module_name() = 0;
    };
}


#endif //ODO_NATIVEMODULE_H
