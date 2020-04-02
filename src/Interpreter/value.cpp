//
// Created by Luis Gonzalez on 3/21/20.
//

#include "Interpreter/value.h"
#include <utility>
#include <vector>

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

void ValueTable::removeReference(Symbol ref) {
    for (auto p : values) {
        Value v = p.second;
        if (existsInSymbols(v.references, ref)) {
            v.removeReference(ref);
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
        if (pm_it->second.references.empty())
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
        removeReference(ref.second);
        if (ref.second.kind == ListType) {
            auto list_val = ref.second.value;

            auto elements = std::any_cast<std::vector<Symbol>>(list_val->val);
            for (auto sym : elements) {
                removeReference(sym);
            }
        }
    }
}

Value* ValueTable::copyValue(const Value& v) {
    return addNewValue(
        v.type,
        v.val
    );
}

void Value::addReference(const Symbol& ref) {
    references.insert(ref);
}

void Value::removeReference(const Symbol& ref) {
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

std::vector<Symbol> Value::as_list_symbol() {
    return std::any_cast< std::vector<Symbol> >(val);
}