//
// Created by louis1001 on 31/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#include "Modules/NativeModule.h"
#include "Interpreter/value.h"
#include "Interpreter/Interpreter.h"
#include "SemAnalyzer/SemanticAnalyzer.h"

namespace Odo::Modules {
    NativeModule::NativeModule(const std::string& name, Interpreting::Interpreter& inter)
        : ModuleValue(nullptr, {name, {}, &inter.get_global()})
        , analyzer(inter.get_analyzer())
    {
        int_type = &inter.get_global().symbols[INT_TP];
        double_type = &inter.get_global().symbols[DOUBLE_TP];
        bool_type = &inter.get_global().symbols[BOOL_TP];
        string_type = &inter.get_global().symbols[STRING_TP];
    }

    void NativeModule::add_literal(const std::string& name, const std::string& value) {
        ownScope.addSymbol({
            .tp = string_type,
            .name = name,
            .value = Interpreting::NormalValue::create(string_type, value),
            .is_initialized = true
        });
    }

    void NativeModule::add_literal(const std::string& name, int value) {
        ownScope.addSymbol({
            .tp = int_type,
            .name = name,
            .value = Interpreting::NormalValue::create(int_type, value),
            .is_initialized = true
        });
    }

    void NativeModule::add_literal(const std::string& name, double value) {
        ownScope.addSymbol({
            .tp = double_type,
            .name = name,
            .value = Interpreting::NormalValue::create(double_type, value),
            .is_initialized = true
        });
    }

    void NativeModule::add_literal(const std::string& name, bool value) {
        ownScope.addSymbol({
            .tp = bool_type,
            .name = name,
            .value = Interpreting::NormalValue::create(bool_type, value),
            .is_initialized = true
        });
    }

    void NativeModule::add_function(
        const std::string &name,
        const std::vector<std::pair<Interpreting::Symbol *, bool>> &arg_types,
        Interpreting::Symbol *ret, std::function<std::any(std::vector<std::any>)> callback
    )
    {
        auto function_type = ownScope.addFuncType(ret, arg_types);

        auto function_symbol = ownScope.addSymbol({
            .tp=function_type,
            .name=name,
            .kind=Interpreting::SymbolType::FunctionSymbol
        });

        auto an = analyzer.lock();
        an->get_function_context_map().insert({function_type, arg_types});

        auto func_value = Interpreting::NativeFunctionValue::create(function_type, arg_types, std::move(callback));
        function_symbol->value = func_value;
        function_symbol->is_initialized = true;
    }
}