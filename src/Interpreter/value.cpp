////
//// Created by Luis Gonzalez on 3/21/20.
////

#include "Interpreter/value.h"
#include "utils.h"
#include <utility>
#include <vector>
#include <iomanip>
#include <sstream>

namespace Odo::Interpreting {
    void Value::addReference(Symbol& sym) {
        references.insert(&sym);
    }

    void Value::removeReference(Symbol& sym) {
        if (references.empty()) return;
        auto indx = references.find(&sym);

        if (indx != references.end()) {
            references.erase(&sym);
        }
    }

    NormalValue::NormalValue(Symbol *tp, std::any the_value) : Value(tp), val(std::move(the_value)) {}

    std::shared_ptr<Value> NormalValue::copy(ValueTable &vt) {
        auto copied_value = std::make_shared<NormalValue>(type, val);

        // Add the new value to the value table.
        vt.addNewValue(copied_value);

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
        if (type->name == "double") {
            auto this_as_double = as_double();

            int floored = floor(this_as_double);
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
        } else if (type->name == "int"){
            auto this_as_int = as_int();
            result = std::to_string(this_as_int);
        } else if (type->name == "string"){
            result = as_string();
        } else if (type->name == "bool"){
            auto this_as_bool = as_bool();
            result = this_as_bool ? "true" : "false";
        } else if (type->name == "NullType"){
            result = "null";
        } else {
            result = "< corrupted :o >";
        }

        return result;
    }

    ListValue::ListValue(Symbol* tp, std::vector<Symbol> sym_elements)
        : Value(tp)
        , elements(std::move(sym_elements)) {}

    std::shared_ptr<Value> ListValue::copy(ValueTable& vt) {
        // Copy elements first.

        auto val_els = as_list_value();
        std::vector<Symbol> symbols_copied;
        for (const auto& value_in_val : val_els) {
            auto list_el = value_in_val;
            if (value_in_val->is_copyable()) {
                list_el = list_el->copy(vt);
            }

            symbols_copied.push_back({value_in_val->type, "list_element", list_el});

            auto& new_sym = *(symbols_copied.end()-1);
            list_el->addReference(new_sym);
            new_sym.value = list_el;
        }

        auto copied_value = std::make_shared<ListValue>(type, std::move(symbols_copied));

        // Add value to the table
        vt.addNewValue(copied_value);

        return copied_value;
    }

    std::string ListValue::to_string() {
        std::string result = "[";
        auto values_in_v = as_list_value();
        for (auto myValIter = values_in_v.begin(); myValIter < values_in_v.end(); myValIter++) {
            auto myVal = *myValIter;
            result += myVal ? myVal->to_string() : "<corrupted>";

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

    FunctionValue::FunctionValue(Symbol* tp, std::vector<std::shared_ptr<Parsing::Node>> params_, std::shared_ptr<Parsing::Node> body_, SymbolTable* scope_)
        : Value(tp)
        , params(std::move(params_))
        , body(std::move(body_))
        , parentScope(scope_) {}

    std::shared_ptr<Value> FunctionValue::copy(ValueTable &vt) {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<FunctionValue>(type, params, body, parentScope);

        // Add the new value to the value table.
        vt.addNewValue(copied_value);

        return copied_value;
    }

    ModuleValue::ModuleValue(Symbol* tp, const SymbolTable& scope)
        : Value(tp)
        , ownScope(scope) {}

    std::shared_ptr<Value> ModuleValue::copy(ValueTable& vt) {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<ModuleValue>(type, ownScope);

        // Add the new value to the value table.
        vt.addNewValue(copied_value);

        return copied_value;
    }

    ClassValue::ClassValue(Symbol* tp, const SymbolTable& scope, SymbolTable* parent_, std::shared_ptr<Parsing::Node> body_)
        : Value(tp)
        , ownScope(scope)
        , body(std::move(body_))
        , parentScope(parent_) {}

    std::shared_ptr<Value> ClassValue::copy(ValueTable& vt) {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<ClassValue>(type, ownScope, parentScope, body);

        // Add the new value to the value table.
        vt.addNewValue(copied_value);

        return copied_value;
    }

    InstanceValue::InstanceValue(Symbol* tp, std::shared_ptr<ClassValue> molde_, const SymbolTable& scope)
        : Value(tp)
        , molde(std::move(molde_))
        , ownScope(scope) {}

    std::shared_ptr<Value> InstanceValue::copy(ValueTable& vt) {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<InstanceValue>(type, molde, ownScope);

        // Add the new value to the value table.
        vt.addNewValue(copied_value);

        return copied_value;
    }

    EnumValue::EnumValue(Symbol* tp, const SymbolTable& scope): Value(tp), ownScope(scope) {}

    std::shared_ptr<Value> EnumValue::copy(ValueTable& vt) {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<EnumValue>(type, ownScope);

        // Add the new value to the value table.
        vt.addNewValue(copied_value);

        return copied_value;
    }

    EnumVarValue::EnumVarValue(Symbol* tp, std::string name_): Value(tp), name(std::move(name_)) {}

    std::shared_ptr<Value> EnumVarValue::copy(ValueTable& vt) {
        // This shouldn't be called ever...
        auto copied_value = std::make_shared<EnumVarValue>(type, name);

        // Add the new value to the value table.
        vt.addNewValue(copied_value);

        return copied_value;
    }

    ValueTable::ValueTable() = default;

    void ValueTable::addNewValue(const std::shared_ptr<Value>& val) {
        int new_index = this->last_index() + 1;
        val->address = new_index;
        values[new_index] = val;
    }

    void ValueTable::removeReference(Symbol& ref) {
        for (const auto& p : values) {
            if (p->references.empty()) continue;
            auto indx = p->references.find(&ref);

            if (indx != p->references.end()) {
                // This is too expensive, apparently. I need to investigate.
                // Also, don't make as many cleanups...
                p->references.erase(indx);
            }
        }
    }

    void ValueTable::cleanUp() {
        auto pm_it = values.begin();
        while(pm_it != values.end())
        {
            if (pm_it->get()->references.empty() && !pm_it->get()->important)
            {
                if (pm_it->get()->kind() == ValueType::ListVal) {
                    auto actual_list = pm_it->get()->as<ListValue>();
                    auto symbols_list = actual_list->elements;

                    for (auto list_ref : symbols_list) {
                        if (list_ref.value) {
                            list_ref.value ->removeReference(list_ref);
                        }

                        removeReference(list_ref);
                    }
                } else if (pm_it->get()->kind() == ValueType::InstanceVal) {
                    auto actual_instance = pm_it->get()->as<InstanceValue>();
                    auto scp = &actual_instance->ownScope;
                    auto parent = scp->getParent();
                    while (parent && starts_with(parent->getName(), "inherited-scope")) {
                        scp = parent->getParent();
                        delete parent;
                        parent = scp;
                    }
                }
                pm_it = values.erase(pm_it);
            }
            else
            {
                ++pm_it;
            }
        }
    }

    void ValueTable::cleanUp(SymbolTable &symTable) {
        for (auto& ref : symTable.symbols) {
            removeReference(ref.second);
        }

        cleanUp();
    }

}