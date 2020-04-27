//
// Created by Luis Gonzalez on 3/21/20.
//

#include "Interpreter/value.h"
#include <utility>
#include <vector>

namespace Odo::Interpreting {
    bool existsInSymbols (std::set<Symbol> arr, Symbol target) {
        auto found = std::find(arr.begin(), arr.end(), target);
        return found != arr.end();
    }

    auto indexInArray (std::set<Symbol> arr, Symbol target) {
        auto found = std::find(arr.begin(), arr.end(), target);
        return found;
    }

    ValueTable::ValueTable() = default;

    Value* ValueTable::addNewValue(Symbol type, std::any val) {
        int newAddress = last_index() + 1;

        Value newValue = {
            std::move(type),
            std::move(val),
            NormalVal,
            newAddress
        };

        values[newAddress] = newValue;

        return &values[newAddress];
    }

    Value* ValueTable::addNewValue(Value val) {
        int newAddress = last_index() + 1;

        Value newValue = {
                std::move(val.type),
                std::move(val.val),
                val.kind,
                newAddress,
                val.scope,
                val.ownScope,
                val.params
        };

        values[newAddress] = newValue;

        return &values[newAddress];
    }

    void ValueTable::removeReference(Symbol ref) {
        for (auto p : values) {
            Value v = p.second;
            auto indx = indexInArray(v.references, ref);

            if (indx != v.references.end()) {
                v.references.erase(ref);
            }
        }
    }

    Value* ValueTable::findFromPointer(int ptr) {
        auto result = values.find(ptr);

        if (result == values.end()) {
            return nullptr;
        } else {
            return &result->second;
        }
    }

    void ValueTable::cleanUp() {
        auto pm_it = values.begin();
        while(pm_it != values.end())
        {
            if (pm_it->second.references.empty() && !pm_it->second.important)
            {
                pm_it = values.erase(pm_it);
            }
            else
            {
                ++pm_it;
            }
        }
    }

    void ValueTable::cleanUp(SymbolTable &symTable) {
        for (auto ref : symTable.symbols) {
            if (ref.second.tp->kind == ListType) {
                if (ref.second.value) {
                    auto symbols_list = std::any_cast<std::vector<Symbol>>(ref.second.value->val);

//                    for (auto list_ref : symbols_list) {
//                        if (list_ref.value) {
//                            list_ref.value ->removeReference(list_ref);
//                        }
//                        /* FIXME: If a value has a reference, but the symbol doesn't
//                         * value is left hanging
//                         */
//                        removeReference(list_ref);
//                    }
                }
            }
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
        auto indx = indexInArray(references, ref);

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
}