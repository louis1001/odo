//
// Created by Luis Gonzalez on 3/21/20.
//

#ifndef ODO_PORT_VALUE_H
#define ODO_PORT_VALUE_H

#include <any>
#include <map>
#include <set>

#include <string>
#include <variant>
#include <Parser/AST.h>

#include "symbol.h"
namespace Odo::Interpreting {
    enum ValueType {
        NormalVal,
        ListVal,
        FunctionVal,
        ModuleVal,
        ClassVal,
        InstanceVal,
        EnumVal,
        EnumVarVal
    };

    struct Value {
        Symbol* type;
        std::any val;

        ValueType kind = NormalVal;
        int address = 0;

        SymbolTable* scope;
        SymbolTable ownScope;
        std::vector<Parsing::AST> params;

        std::set<Symbol*> references;
        bool important = false;

        void addReference(Symbol &ref);
        void removeReference(Symbol &ref);

        bool as_bool();
        std::string as_string();
        int as_int();
        double as_double();
        std::vector<Value*> as_list_value();
        std::vector<Symbol>& as_list_symbol();

        std::string to_string();

        [[nodiscard]] bool is_copyable() const {
            return kind == ListVal || (type && type->kind == PrimitiveType && type->name != "NullType");
        }

        [[nodiscard]] bool is_numeric() const { return type->name == "int" || type->name == "double"; }
    };

    class ValueTable {
        std::map<int, Value> values;

        int last_index() {
            return values.empty() ?
            -1 :
            (--values.end())->first;
        }

        Value* copyListValue(Value);

    public:
        ValueTable();

        Value* addNewValue(Symbol* type, std::any val);
        Value* addNewValue(const Value&);

        void removeReference(Symbol &ref);

//        Value* findFromPointer(int ptr);

        void cleanUp();
        void cleanUp(SymbolTable &symTable);

        Value* copyValue(const Value&);
    };
}
#endif //ODO_PORT_VALUE_H
