//
// Created by Luis Gonzalez on 3/21/20.
//

#ifndef ODO_PORT_SYMBOL_H
#define ODO_PORT_SYMBOL_H
#include <string>
#include <map>

struct Value;

std::string random_string(size_t length);

enum SymbolType {
    Var,
    ListType,
    PrimitiveType,
    ClassType,
    FunctionType,
    FunctionSymbol,
};

struct Symbol {
    Symbol *tp = nullptr;
    std::string name = "";
    Value* value = nullptr;
    bool isType = false;
    SymbolType kind = Var;

    std::string uuid = random_string(9);

    bool operator==(const Symbol& rhs) const { return (this->name + this->uuid) == (rhs.name + rhs.uuid); }
};

namespace std
{
    template<> struct less<Symbol>
    {
        bool operator() (const Symbol& lhs, const Symbol& rhs) const
        {
            return (lhs.name + lhs.uuid) > (rhs.name + rhs.uuid);
        }
    };
}

class SymbolTable {
    SymbolTable *parent{};

    std::string scopeName = "";
    int level=0;

public:
    SymbolTable();
    SymbolTable(std::string, std::map<std::string, Symbol> types, SymbolTable *parent= nullptr);

    std::map<std::string, Symbol> symbols;

    Symbol* findSymbol(const std::string&);
    Symbol* addSymbol(Symbol);
    bool symbolExists(const std::string&);

    SymbolTable* getParent() { return parent; }
};
#endif //ODO_PORT_SYMBOL_H
