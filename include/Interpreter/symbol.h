//
// Created by Luis Gonzalez on 3/21/20.
//

#ifndef ODO_PORT_SYMBOL_H
#define ODO_PORT_SYMBOL_H
#include <string>
#include <map>
#include <vector>
#include <limits>
#include "utils.h"

#include "Lexer/token.hpp"
namespace Odo::Interpreting {
    struct Value;

    enum SymbolType {
        Var,
        ListType,
        PrimitiveType,
        ClassType,
        FunctionType,
        FunctionSymbol,
        EnumType,
        EnumsSymbol
    };

    /* So here's the thing...
     * When I started this project I didn't have much knowledge about
     * C++.
     * I was getting mixed signals. When I decided I wanted to do something
     * With polymorphism, casting and type checking a Symbol, for example,
     * it wasn't as simple as in C#, so I investigated.
     * And what I found was people saying "But why would you want that? Then don't
     * use polymorphism... Why would you want to store a <my own example> FunctionSymbol
     * in a Symbol variable just to cast it back to FunctionSymbol? That defeats the purpose
     * of inheritance!". And me, being me, thought "well, I'm a noob. This people know better.
     * Let's not use polymorphism." And that's how I ended up with this monster of a Symbol struct.
     *
     * Now, a couple of weeks into the project, and after watching some people use C++,
     * I discover that using static_cast<ChildType> is completly valid.
     *
     * So I'll refactor this in a moment. Just... keep that in mind for the moment.
     *
     *  *Abandon all hope, ye who enter here*
     */


    struct Symbol {
        Symbol *tp = nullptr;
        std::string name = "";
        Value* value = nullptr;
        bool isType = false;
        SymbolType kind = Var;
        std::vector< std::pair<Symbol, bool> > functionTypes;

        int uuid = rand_int(0, 1e8);

        bool operator==(const Symbol& rhs) const { return this->uuid == rhs.uuid; }

        bool operator<(const Odo::Interpreting::Symbol& rhs) const { return uuid > rhs.uuid; }

        static std::string constructFuncTypeName(Symbol* type, const std::vector< std::pair<Symbol, bool> >& paramTypes) {
            std::string result = "(";
            auto cont = 0;
            for (const auto& arg : paramTypes) {
                result += arg.first.name + (arg.second ? "" : "?");
                if (cont != paramTypes.size()-1) {
                    result += ", ";
                }
                cont++;
            }

            result += ")";

            if (type) {
                result += ": " + type->name;
            }

            return result;
        }
    };

    class SymbolTable {
        SymbolTable *parent{};

        std::string scopeName = "";
        int level=0;

    public:
        SymbolTable();
        SymbolTable(std::string, std::map<std::string, Symbol> types, SymbolTable *parent= nullptr);

        std::map<std::string, Symbol> symbols;

        Symbol* findSymbol(const std::string&, bool and_in_parents=true);
        Symbol* addSymbol(const Symbol&);
        Symbol* addListType(Symbol*);
        bool symbolExists(const std::string&);

        std::string getName() { return scopeName; }

        SymbolTable* getParent() { return parent; }
        void setParent(SymbolTable* newP) { parent = newP; }

        Symbol *addFuncType(Symbol *pSymbol, const std::vector<std::pair<Symbol, bool>>& vector);

        void debugChain();
        ~SymbolTable();
    };
}
#endif //ODO_PORT_SYMBOL_H
