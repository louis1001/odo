//
// Created by Luis Gonzalez on 3/21/20.
//

#include "Interpreter/symbol.h"
#include "Exceptions/exception.h"
#include "Translations/lang.h"
#include <utility>
#include <iostream>

namespace Odo::Interpreting {
    SymbolTable::SymbolTable() = default;

    SymbolTable::SymbolTable(std::string name_, std::unordered_map<std::string, Symbol> types_, SymbolTable *parent_) {
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
            if (!aliases.empty()) {
                auto in_aliases = aliases.find(name);
                if(in_aliases != aliases.end()) return in_aliases->second;
            }

            if (and_in_parents && parent != nullptr){
                return parent->findSymbol(name);
            }

            return nullptr;
        } else {
            return &foundS->second;
        }
    }

    Symbol* SymbolTable::addSymbol(const Symbol& sym) {
        std::string new_sym_name = sym.name;
        auto foundS = symbols.find(new_sym_name);

        if (foundS != symbols.end()) {
            throw Exceptions::NameException(REDEF_SYM_EXCP + new_sym_name);
        }

        if (sym.tp != nullptr) {
            if (!sym.tp->isType) {
                throw Exceptions::TypeException(SYM_TXT_EXCP + new_sym_name + NOT_TYPE_EXCP);
            }
        }

        symbols[new_sym_name] = sym;

        return &symbols.find(new_sym_name)->second;
    }

    Symbol* SymbolTable::addAlias(const std::string& name, Symbol* sym) {
        if (symbolExists(name)) {
            return nullptr;
        }
        aliases.insert({name, sym});

        return sym;
    }

    Symbol* SymbolTable::addAlias(const std::string& name, const std::string& syn_name) {
        if (symbolExists(name)) {
            return nullptr;
        }
        auto foundS = aliases.find(name);

        aliases.insert({name, foundS->second});
        return foundS->second;
    }

    void SymbolTable::removeSymbol(Symbol* name) {
        for (auto it = symbols.begin(); it != symbols.end(); it++) {
            if (&it->second == name) {
                symbols.erase(it);
                return;
            }
        }
    }

    Symbol* SymbolTable::addListType(Symbol* tp) {
        std::string new_sym_name = tp->name+"[]";
        auto foundAsListType = symbols.find(new_sym_name);
        if (foundAsListType != symbols.end())
            return &foundAsListType->second;

        auto foundS = symbols.find(tp->name);

        if (foundS != symbols.end()) {
            if (!foundS->second.isType) {
                throw Exceptions::TypeException(SYM_TXT_EXCP + foundS->second.name + NOT_TYPE_EXCP);
            }
        }

        symbols[new_sym_name] = Symbol{
            .tp=tp,
            .name=new_sym_name,
            .isType=true,
            .kind=SymbolType::ListType,
        };

        return &symbols.find(new_sym_name)->second;
    }

    bool SymbolTable::symbolExists(const std::string& name) {
        auto in_symbols = symbols.find(name);

        if (in_symbols != symbols.end()) return true;
        return aliases.find(name) != aliases.end();
    }

    Symbol *SymbolTable::addFuncType(Symbol *type, const std::vector<std::pair<Symbol*, bool>>& params) {
        auto funcName = Symbol::constructFuncTypeName(type, params);

        return addFuncType(type, funcName);
    }

    Symbol *SymbolTable::addFuncType(Symbol *type, const std::string& funcName) {
        auto foundAsFuncType = symbols.find(funcName);
        if (foundAsFuncType != symbols.end())
            return &foundAsFuncType->second;

        if (type){
            auto foundS = symbols.find(type->name);

            if (foundS != symbols.end()) {
                if (!foundS->second.isType) {
                    throw Exceptions::TypeException(SYM_TXT_EXCP + foundS->second.name + NOT_TYPE_EXCP);
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

        return &symbols.find(funcName)->second;
    }

    void SymbolTable::debugChain() {
        std::cout << TABLE_LEV_MSG << level << ": " << scopeName <<
            " | " WITH_MSG << symbols.size() << SYMS_MSG;
        if (parent) {
            std::cout << "\t|\n\tv\n";
            parent->debugChain();
        }
    }

}