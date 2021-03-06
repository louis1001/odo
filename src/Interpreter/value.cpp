////
//// Created by Luis Gonzalez on 3/21/20.
////

#include "Interpreter/value.h"
#include "utils.h"
#include <utility>
#include <vector>
#include <iomanip>
#include <sstream>
#include <Interpreter/Interpreter.h>
#include "Translations/lang.h"

namespace Odo::Interpreting {

    NormalValue::NormalValue(Symbol *tp, std::any the_value) : Value(tp), val(std::move(the_value)) {}

    std::shared_ptr<Value> NormalValue::copy() {
        auto copied_value = std::make_shared<NormalValue>(type, val);

        return copied_value;
    }

    int NormalValue::as_int() {
        return std::any_cast<int>(val);
    }

    double NormalValue::as_double() {
        return std::any_cast<double>(val);
    }

    bool NormalValue::as_bool() {
        return std::any_cast<bool>(val);
    }

    std::string NormalValue::as_string() {
        return std::any_cast<std::string>(val);
    }

    std::string NormalValue::to_string() {
        std::string result;
        if (type->name == DOUBLE_TP) {
            auto this_as_double = as_double();

            int floored = static_cast<int>(floor(this_as_double));
            if (this_as_double - floored < 1e-15) {
                result = std::to_string(floored) + ".0";
            } else {
                std::stringstream double_parsed;
                double_parsed << std::setprecision(std::numeric_limits<double>::digits10) << this_as_double;

                result = double_parsed.str();
                result.erase ( result.find_last_not_of('0') + 1, std::string::npos );
                if (result.find('.') == std::string::npos){
                    result += ".0";
                }
            }
        } else if (type->name == INT_TP){
            auto this_as_int = as_int();
            result = std::to_string(this_as_int);
        } else if (type->name == STRING_TP){
            result = as_string();
        } else if (type->name == BOOL_TP){
            auto this_as_bool = as_bool();
            result = this_as_bool ? TRUE_TK : FALSE_TK;
        } else if (type->name == NULL_TP){
            result = NULL_TK;
        } else {
            result = CORRUPTED_MSG;
        }

        return result;
    }

    std::shared_ptr<NormalValue> NormalValue::create(Symbol *tp, std::any the_value) {
        return std::make_shared<NormalValue>(tp, the_value);
    }

    ListValue::ListValue(Symbol* tp, std::vector<Symbol> sym_elements)
        : Value(tp)
        , elements(std::move(sym_elements)) {}

    std::shared_ptr<ListValue> ListValue::create(Symbol* tp, std::vector<Symbol> sym_elements) {
        return std::make_shared<ListValue>(tp, sym_elements);
    }

    std::shared_ptr<Value> ListValue::copy() {
        // Copy elements first.

        auto val_els = as_list_value();
        std::vector<Symbol> symbols_copied;
        for (const auto& value_in_val : val_els) {
            auto list_el = value_in_val;
            if (value_in_val->is_copyable()) {
                list_el = list_el->copy();
            }

            symbols_copied.push_back({value_in_val->type, "list_element", list_el});

            auto& new_sym = *(symbols_copied.end()-1);
            new_sym.value = list_el;
        }

        auto copied_value = std::make_shared<ListValue>(type, std::move(symbols_copied));

        return copied_value;
    }

    std::string ListValue::to_string() {
        std::string result = "[";
        auto values_in_v = as_list_value();
        for (auto myValIter = values_in_v.begin(); myValIter < values_in_v.end(); myValIter++) {
            auto myVal = *myValIter;
            result += myVal ? myVal->to_string() : CORRUPTED_MSG;

            if (myValIter != values_in_v.end() - 1) {
                result += ", ";
            }
        }
        result += "]";

        return result;
    }

    std::vector<std::shared_ptr<Value> > ListValue::as_list_value() {
        std::vector<std::shared_ptr<Value>> results;
        results.reserve(elements.size());
        for (const auto& s : elements) {
            results.push_back(std::shared_ptr<Value>(s.value));
        }

        return results;
    }

    FunctionValue::FunctionValue(Symbol* tp, std::vector<std::shared_ptr<Parsing::Node>> params_, std::shared_ptr<Parsing::Node> body_, SymbolTable* scope_, std::string name_)
        : Value(tp)
        , params(std::move(params_))
        , body(std::move(body_))
        , parentScope(scope_)
        , name(std::move(name_)) {}


    std::shared_ptr<FunctionValue> FunctionValue::create(Symbol* tp, std::vector<std::shared_ptr<Parsing::Node>> params_, std::shared_ptr<Parsing::Node> body_, SymbolTable* scope_, std::string name) {
        return std::make_shared<FunctionValue>(tp, params_, body_, scope_, name=ANONYMUS_MSG);
    }

    std::shared_ptr<Value> FunctionValue::copy() {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<FunctionValue>(type, params, body, parentScope, name);

        return copied_value;
    }

    ModuleValue::ModuleValue(Symbol* tp, const SymbolTable& scope)
        : Value(tp)
        , ownScope(scope) {}


    std::shared_ptr<ModuleValue> ModuleValue::create(Symbol* tp, const SymbolTable& scope) {
        return std::make_shared<ModuleValue>(tp, scope);
    }

    std::shared_ptr<Value> ModuleValue::copy() {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<ModuleValue>(type, ownScope);

        return copied_value;
    }

    ClassValue::ClassValue(Symbol* tp, const SymbolTable& scope, SymbolTable* parent_, std::shared_ptr<Parsing::Node> body_)
        : Value(tp)
        , ownScope(scope)
        , body(std::move(body_))
        , parentScope(parent_) {}

    std::shared_ptr<ClassValue> ClassValue::create(Symbol* tp, const SymbolTable& scope, SymbolTable* parent_, std::shared_ptr<Parsing::Node> body_) {
        return std::make_shared<ClassValue>(tp, scope, parent_, body_);
    }

    std::shared_ptr<Value> ClassValue::copy() {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<ClassValue>(type, ownScope, parentScope, body);

        return copied_value;
    }

    Symbol* ClassValue::getStaticVarSymbol(const std::string& name) {
        auto inMe = ownScope.findSymbol(name);

        if (inMe) return inMe;

        if (type && type->tp && type->tp->value) {
            auto myParent = as<ClassValue>(type->tp->value);
            if (myParent) {
                return myParent->getStaticVarSymbol(name);
            }
        }

        return nullptr;
    }

    InstanceValue::InstanceValue(Symbol* tp, std::shared_ptr<ClassValue> molde_, const SymbolTable& scope)
        : Value(tp)
        , molde(std::move(molde_))
        , ownScope(scope) {}

    std::shared_ptr<InstanceValue> InstanceValue::create(Symbol* tp, std::shared_ptr<ClassValue> molde_, const SymbolTable& scope) {
        return std::make_shared<InstanceValue>(tp, molde_, scope);
    }

    std::shared_ptr<Value> InstanceValue::copy() {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<InstanceValue>(type, molde, ownScope);

        return copied_value;
    }

    Symbol* InstanceValue::getStaticVarSymbol(const std::string& name) {
        return molde->getStaticVarSymbol(name);
    }

    EnumValue::EnumValue(Symbol* tp, const SymbolTable& scope): Value(tp), ownScope(scope) {}

    std::shared_ptr<EnumValue> EnumValue::create(Symbol* tp, const SymbolTable& scope) {
        return std::make_shared<EnumValue>(tp, scope);
    }

    std::shared_ptr<Value> EnumValue::copy() {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<EnumValue>(type, ownScope);

        return copied_value;
    }

    EnumVarValue::EnumVarValue(Symbol* tp, std::string name_): Value(tp), name(std::move(name_)) {}

    std::shared_ptr<EnumVarValue> EnumVarValue::create(Symbol* tp, std::string name_) {
        return std::make_shared<EnumVarValue>(tp, name_);
    }

    std::shared_ptr<Value> EnumVarValue::copy() {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<EnumVarValue>(type, name);

        return copied_value;
    }

    std::shared_ptr<Value> NativeFunctionValue::copy() {
        auto copied_value = std::make_shared<NativeFunctionValue>(type, arguments, fn);
        return copied_value;
    }

    NativeFunctionValue::NativeFunctionValue(Symbol* tp, std::vector<std::pair<Symbol*, bool>> args, simple_primitives_function_type fn_)
        : Value(tp), fn(std::move(fn_)), arguments(std::move(args)), function_kind(NativeFunctionType::Simple) {}

    NativeFunctionValue::NativeFunctionValue(Symbol* tp, std::vector<std::pair<Symbol*, bool>> args, handle_values_function_type fn_)
        : Value(tp), values_fn(std::move(fn_)), arguments(std::move(args)), function_kind(NativeFunctionType::Values) {}

    std::shared_ptr<NativeFunctionValue> NativeFunctionValue::create(Symbol* tp, const std::vector<std::pair<Symbol*, bool>>& args, const simple_primitives_function_type& fn_) {
        return std::make_shared<NativeFunctionValue>(tp, args, std::move(fn_));
    }

    std::shared_ptr<NativeFunctionValue> NativeFunctionValue::create(Symbol* tp, const std::vector<std::pair<Symbol*, bool>>& args, const handle_values_function_type& fn_) {
        return std::make_shared<NativeFunctionValue>(tp, args, fn_);
    }
}