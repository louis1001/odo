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
#include <Parser/AST/Node.h>

#include "symbol.h"
namespace Odo::Interpreting {
    class ValueTable;

    enum class ValueType {
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
        Symbol* type {nullptr};
        int address {0};
        bool important{false};
        std::set<Symbol*> references;

        virtual ValueType kind()=0;
        virtual std::shared_ptr<Value> copy(ValueTable&)=0;
        [[nodiscard]] virtual bool is_numeric() const { return false; }
        [[nodiscard]] virtual bool is_copyable() const { return false; }

        virtual std::string to_string() { return "<value> at: " + std::to_string(address); }

        virtual void addReference(Symbol&);
        virtual void removeReference(Symbol&);

        template<typename T>
        static std::shared_ptr<T> as(const std::shared_ptr<Value>& v) {
            return std::dynamic_pointer_cast<T>(v);
        }

        explicit Value(Symbol* sym, int add=0): type(sym), address(add) { }
    };

    struct NormalValue final: public Value {
        std::any val;

        ValueType kind() final { return ValueType::NormalVal; }
        [[nodiscard]] bool is_numeric() const final { return type->name == "int" || type->name == "double"; }
        [[nodiscard]] bool is_copyable() const final { return type->name != "NullType"; }

        std::shared_ptr<Value> copy(ValueTable& vt) final;


        int as_int();
        double as_double();
        bool as_bool();
        std::string as_string();

        std::string to_string() final;
        NormalValue(Symbol *tp, std::any the_value);
    };

    struct ListValue final: public Value {
        std::vector<Symbol> elements;
        ValueType kind() final { return ValueType::ListVal; }
        [[nodiscard]] bool is_copyable() const final { return true; }

        std::shared_ptr<Value> copy(ValueTable& vt) final;

        std::string to_string() final;
        std::vector<std::shared_ptr<Value>> as_list_value();
        ListValue(Symbol* tp, std::vector<Symbol> sym_elements);
    };

    struct FunctionValue final: public Value {
        std::vector<std::shared_ptr<Parsing::Node>> params;
        std::shared_ptr<Parsing::Node> body;
        SymbolTable* parentScope{};
        ValueType kind() final { return ValueType::FunctionVal; }

        std::shared_ptr<Value> copy(ValueTable& vt) final;

        std::string to_string() final { return "<function> at: " + std::to_string(address); }

        FunctionValue(Symbol* tp, std::vector<std::shared_ptr<Parsing::Node>> params_, std::shared_ptr<Parsing::Node> body_, SymbolTable* scope_);
    };

    struct ModuleValue final: public Value {
        SymbolTable ownScope;
        ValueType kind() final { return ValueType::ModuleVal; }

        std::shared_ptr<Value> copy(ValueTable& vt) final;

        std::string to_string() final { return "<module> at: " + std::to_string(address); }

        ModuleValue(Symbol* tp, const SymbolTable& scope);
    };

    struct ClassValue final: public Value {
        SymbolTable ownScope;
        std::shared_ptr<Parsing::Node> body;
        // What's the point of the parent scope if ownscope has it?
        SymbolTable* parentScope;
        ValueType kind() final { return ValueType::ClassVal; }

        std::shared_ptr<Value> copy(ValueTable& vt) final;

        std::string to_string() final { return "<class> at: " + std::to_string(address); }

        ClassValue(Symbol* tp, const SymbolTable& scope, SymbolTable* parent_, std::shared_ptr<Parsing::Node> body_);
    };

    struct InstanceValue final: public Value {
        std::shared_ptr<ClassValue> molde;
        SymbolTable ownScope;

        ValueType kind() final { return ValueType::InstanceVal; }

        std::shared_ptr<Value> copy(ValueTable& vt) final;

        std::string to_string() final { return "<instance> at: " + std::to_string(address); }

        InstanceValue(Symbol* tp, std::shared_ptr<ClassValue> molde_, const SymbolTable& scope);
    };

    struct EnumValue final: public Value {
        SymbolTable ownScope;
        ValueType kind() final { return ValueType::EnumVal; }

        std::shared_ptr<Value> copy(ValueTable& vt) final;

        std::string to_string() final { return "<enum> at: " + std::to_string(address); }

        EnumValue(Symbol* tp, const SymbolTable& scope);
    };

    struct EnumVarValue final: public Value {
        std::string name;
        ValueType kind() final { return ValueType::EnumVarVal; }

        std::shared_ptr<Value> copy(ValueTable& vt) final;

        std::string to_string() final { return name; }

        EnumVarValue(Symbol* tp, std::string name_);
    };

    class ValueTable {
        std::vector<std::shared_ptr<Value>> values;

    public:
        int last_index() {
            if (values.empty()) return -1;
            return (--values.end())->get()->address;
        }
        ValueTable();

        void addNewValue(const std::shared_ptr<Value>&);
        void removeReference(Symbol& ref);

        void cleanUp();
        void cleanUp(SymbolTable &symTable);
    };
}
#endif //ODO_PORT_VALUE_H
