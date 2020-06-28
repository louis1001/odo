//
// Created by Luis Gonzalez on 3/21/20.
//

#include "Interpreter/value.h"
#include "utils.h"
#include <utility>
#include <vector>
#include <iomanip>
#include <sstream>

namespace Odo::Interpreting {
    ValueTable::ValueTable() = default;

    Value* ValueTable::addNewValue(Symbol* type, std::any val) {
        int newAddress = last_index() + 1;

        Value newValue = {
            type,
            std::move(val),
            NormalVal,
            newAddress
        };

        values[newAddress] = newValue;

        return &values[newAddress];
    }

    Value* ValueTable::addNewValue(const Value& val) {
        int newAddress = last_index() + 1;

        Value newVal = {
                val.type,
                val.val,
                val.kind,
                newAddress,
                val.scope,
                val.ownScope,
                val.params
        };

        values[newAddress] = std::move(newVal);

        return &values[newAddress];
    }

    void ValueTable::removeReference(const Symbol& ref) {
        for (const auto& p : values) {
            if (p.second.references.empty()) continue;
            Value v = p.second;
            auto indx = v.references.find(ref);

            if (indx != v.references.end()) {
                // This is too expensive, apparently. I need to investigate.
                // Also, don't make as many cleanups...
                v.references.erase(indx);
            }
        }
    }

    void ValueTable::cleanUp() {
        auto pm_it = values.begin();
        while(pm_it != values.end())
        {
            if (pm_it->second.references.empty() && !pm_it->second.important)
            {
                if (pm_it->second.kind == ListVal) {
                    auto symbols_list = pm_it->second.as_list_symbol();

                    for (auto list_ref : symbols_list) {
                        if (list_ref.value) {
                            list_ref.value ->removeReference(list_ref);
                        }

                        removeReference(list_ref);
                    }
                } else if (pm_it->second.kind == InstanceVal) {
                    auto scp = &pm_it->second.ownScope;
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
        for (const auto& ref : symTable.symbols) {
            removeReference(ref.second);
        }

        cleanUp();
    }

    Value* ValueTable::copyValue(const Value& v) {
        return addNewValue(v);
    }

    void Value::addReference(const Symbol& ref) {
        references.insert(ref);
    }

    void Value::removeReference(Symbol &ref) {
        if (references.empty()) return;
        auto indx = references.find(ref);

        if (indx != references.end()) {
            references.erase(ref);
        }
    }

    bool Value::as_bool() {
        return std::any_cast<bool>(val);
    }

    std::string Value::as_string() {
        return std::any_cast<std::string>(val);
    }

    int Value::as_int() {
        return std::any_cast<int>(val);
    }

    double Value::as_double() {
        return std::any_cast<double>(val);
    }

    std::vector<Value*> Value::as_list_value() {
        auto list_symbols = std::any_cast< std::vector<Symbol> >(val);

        std::vector<Value*> results;
        results.reserve(list_symbols.size());
        for (const auto& s : list_symbols) {
            results.push_back(s.value);
        }
        return results;
    }

    std::vector<Symbol>& Value::as_list_symbol() {
        return std::any_cast< std::vector<Symbol>& >(val);
    }

    std::string Value::to_string() {
        std::string result;
        if (kind == ModuleVal) {
            result = "<module> at: " + std::to_string(address);
        } else if (type && type->kind == PrimitiveType){
            if (type->name == "double") {
                auto this_as_double = as_double();

                std::stringstream double_parsed;
                double_parsed << std::setprecision(std::numeric_limits<double>::digits10+1) << this_as_double;

                result = double_parsed.str();
                result.erase ( result.find_last_not_of('0') + 1, std::string::npos );
                if (*(result.end()-1) == '.') result += "0";
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
        } else if (kind == ListVal) {
            result += "[";
            auto values_in_v = std::any_cast<std::vector<Symbol> >(val);
            for (auto mySym = values_in_v.begin(); mySym < values_in_v.end(); mySym++) {
                result += mySym->value ? mySym->value->to_string() : "null";

                if (mySym != values_in_v.end() - 1) {
                    result += ", ";
                }
            }
            result += "]";
        } else if (kind == ClassVal){
            result = "<class> at: " + std::to_string(address);
        } else if (kind == InstanceVal){
            result = "<instance> at: " + std::to_string(address);
        } else if (kind == FunctionVal){
            result = "<function> at: " + std::to_string(address);
        } else if (kind == EnumVal){
            result = "<enum> at: " + std::to_string(address);
        } else if (kind == EnumVarVal){
            result = as_string();
        } else {
            result = "<value> at: " + std::to_string(address);
        }

        return result;
    }
}