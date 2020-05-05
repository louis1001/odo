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
        InstanceVal
    };

    struct Value {
        Symbol type;
        std::any val;

        ValueType kind = NormalVal;
        int address = 0;

        SymbolTable* scope;
        SymbolTable ownScope;
        std::vector<Parsing::AST> params;

        std::set<Symbol> references;
        bool important = false;

        void addReference(const Symbol& ref);
        void removeReference(Symbol &ref);

        bool as_bool();
        std::string as_string();
        int as_int();
        double as_double();
        std::vector<Value*> as_list_value();
        std::vector<Symbol>& as_list_symbol();
    };

    class ValueTable {
        std::map<int, Value> values;

        int last_index() {
            return values.empty() ?
            0 :
            (--values.end())->first;
        }

    public:
        ValueTable();

        Value* addNewValue(Symbol type, std::any val);
        Value* addNewValue(Value);

        void removeReference(Symbol ref);

//        Value* findFromPointer(int ptr);

        void cleanUp();
        void cleanUp(SymbolTable &symTable);

        Value* copyValue(const Value&);
    };
}
#endif //ODO_PORT_VALUE_H
