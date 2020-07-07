//
// Created by Luis Gonzalez on 3/21/20.
//

#include "Interpreter/symbol.h"
#include "Exceptions/exception.h"
#include <utility>
#include <iostream>

namespace Odo::Interpreting {
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

    Symbol *SymbolTable::findSymbol(const std::string& name, bool and_in_parents) {
        auto foundS = symbols.find(name);

        if (foundS == symbols.end()) {
            if (and_in_parents && parent != nullptr){
                return parent->findSymbol(name);
            } else {
                return nullptr;
            }
        } else {
            return &foundS->second;
        }
    }

    Symbol* SymbolTable::addSymbol(const Symbol& sym) {
        auto foundS = symbols.find(sym.name);

        if (foundS != symbols.end()) {
            throw Exceptions::NameException("Redefinition of symbol " + sym.name);
        }

        if (sym.tp != nullptr) {
            if (!sym.tp->isType) {
                throw Exceptions::TypeException("Symbol '" + sym.name + "' is not a type.");
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
                throw Exceptions::TypeException("Symbol '" + foundS->second.name + "' is not a type.");
            }
        }

        symbols[tp->name+"[]"] = Symbol{
            .tp=tp,
            .name=tp->name+"[]",
            .isType=true,
            .kind=SymbolType::ListType,
        };

        return &symbols[tp->name + "[]"];
    }

    bool SymbolTable::symbolExists(const std::string& name) {
        return symbols.find(name) != symbols.end();
    }

    Symbol *SymbolTable::addFuncType(Symbol *type, const std::vector<std::pair<Symbol, bool>>& params) {
        auto funcName = Symbol::constructFuncTypeName(type, params);

        auto foundAsFuncType = symbols.find(funcName);
        if (foundAsFuncType != symbols.end())
            return &foundAsFuncType->second;

        if (type){
            auto foundS = symbols.find(type->name);

            if (foundS != symbols.end()) {
                if (!foundS->second.isType) {
                    throw Exceptions::TypeException("Symbol '" + foundS->second.name + "' is not a type.");
                }
            }
        }

        // Where should the param types go?
        // In a member var of symbol, apparently... Ugh.
        symbols[funcName] = Symbol{
            .tp=type,
            .name=funcName,
            .isType=true,
            .kind=SymbolType::FunctionType
        };

        return &symbols[funcName];
    }

    void SymbolTable::debugChain() {
        std::cout << "Table level " << level << ": " << scopeName <<
            " | with " << symbols.size() << " symbols\n";
        if (parent) {
            std::cout << "\t|\n\tv\n";
            parent->debugChain();
        }
    }

    SymbolTable::~SymbolTable() {
        symbols.clear();
    }

}