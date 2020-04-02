//
// Created by Luis Gonzalez on 3/21/20.
//

#include "Interpreter/symbol.h"
#include <utility>

const static char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

std::string random_string(size_t length) {
    // Taken from:
    // https://stackoverflow.com/a/12468109
    auto randchar = []() -> char
    {
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

SymbolTable::SymbolTable() = default;

SymbolTable::SymbolTable(std::string name_, std::map<std::string, Symbol> types_, SymbolTable *parent_) {
    scopeName = std::move(name_);
    symbols = std::move(types_);
    parent = parent_;

    if (parent_ != nullptr) {
        level = parent_->level + 1;
    } else {
        level = 0;
    }
}

Symbol *SymbolTable::findSymbol(const std::string& name) {
    auto foundS = symbols.find(name);

    if (foundS == symbols.end()) {
        if (parent != nullptr){
            return parent->findSymbol(name);
        } else {
            return nullptr;
        }
    } else {
        return &foundS->second;
    }
}

Symbol* SymbolTable::addSymbol(Symbol sym) {
    auto foundS = symbols.find(sym.name);

    if (foundS != symbols.end()) {
        // TODO: Handle errors
        // NameError! Redefinition of symbol
        throw 1;
    }

    if (sym.tp != nullptr) {
        if (!sym.tp->isType) {
            // TODO: Handle errors
            // TypeError! Symbol(identifier) [sym.tp.name] is not a type
            throw 1;
        }
    }

    symbols[sym.name] = sym;

    return &symbols[sym.name];
}

Symbol* SymbolTable::addListType(Symbol* tp) {
    auto foundAsListType = symbols.find(tp->name+"[]");
    if (foundAsListType != symbols.end())
        return &foundAsListType->second;

    auto foundS = symbols.find(tp->name);

    if (foundS != symbols.end()) {
        if (!foundS->second.isType) {
            // TODO: Handle errors
            // TypeError! Symbol(identifier) [sym.tp.name] is not a type
            throw 1;
        }
    }

    symbols[tp->name+"[]"] = Symbol{
        tp,
        tp->name+"[]",
        .kind=ListType,
        .isType=true
    };

    return &symbols[tp->name + "[]"];
}

bool SymbolTable::symbolExists(const std::string& name) {
    return symbols.find(name) != symbols.end();
}

