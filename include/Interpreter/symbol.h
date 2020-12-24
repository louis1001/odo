//
// Created by Luis Gonzalez on 3/21/20.
//

#ifndef ODO_PORT_SYMBOL_H
#define ODO_PORT_SYMBOL_H
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include "utils.h"
#include <memory>
#include <functional>

#include "Translations/lang.h"

#include "Lexer/token.hpp"
namespace Odo::Interpreting {
    struct Value;

    enum class SymbolType {
        VarSymbol,
        ListType,
        PrimitiveType,
        ClassType,
        FunctionType,
        FunctionSymbol,
        EnumType,
        EnumSymbol,
        ModuleSymbol
    };

    /*
     * I'm fixing the jankiness
     *  *Abandon all hope, ye who enter here*
     */


    struct Symbol {
        Symbol *tp = nullptr;
        std::string name = "";
        std::shared_ptr<Value> value = nullptr;
        bool isType = false;
        SymbolType kind = SymbolType::VarSymbol;

        bool is_initialized{false};
        bool content_is_constant{false};
        bool content_has_side_effects{false};

        std::function<void(Symbol*)> ondestruction{nullptr};

        [[nodiscard]] bool is_numeric() const { return name == INT_TP || name == DOUBLE_TP; }

        static std::string constructFuncTypeName(Symbol* type, const std::vector< std::pair<Symbol*, bool> >& paramTypes) {
            std::string result = "(";
            auto cont = 0;
            for (const auto& arg : paramTypes) {
                result += arg.first->name + (arg.second ? "?" : "");
                if (cont != paramTypes.size()-1) {
                    result += ", ";
                }
                cont++;
            }

            result += ")";

            result += ": " + (type? type->name : "void");

            return result;
        }

        ~Symbol() {
            if (ondestruction) {
                ondestruction(this);
            }
        }
    };

    class SymbolTable {
        SymbolTable *parent{};

        std::string scopeName = "";
        int level=0;

    public:
        SymbolTable();
        SymbolTable(std::string, std::unordered_map<std::string, Symbol> types, SymbolTable *parent= nullptr);

        std::unordered_map<std::string, Symbol> symbols;

        Symbol* findSymbol(const std::string&, bool and_in_parents=true);
        Symbol* addSymbol(const Symbol&);
        Symbol* addListType(Symbol*);
        bool symbolExists(const std::string&);

        std::string getName() { return scopeName; }

        SymbolTable* getParent() { return parent; }
        void setParent(SymbolTable* newP) { parent = newP; }

        Symbol *addFuncType(Symbol *pSymbol, const std::vector<std::pair<Symbol*, bool>>& vector);
        Symbol *addFuncType(Symbol *pSymbol, const std::string& funcName);

        void debugChain();
        ~SymbolTable();
    };
}
#endif //ODO_PORT_SYMBOL_H
