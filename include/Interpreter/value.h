//
// Created by Luis Gonzalez on 3/21/20.
//

#ifndef ODO_PORT_VALUE_H
#define ODO_PORT_VALUE_H

#include <any>
#include <map>
#include <set>
#include <sstream>

#include <string>
#include <variant>
#include <Parser/AST/Node.h>
#include <functional>
#include "Translations/lang.h"

#include "symbol.h"
namespace Odo::Interpreting {

    enum class ValueType {
        NormalVal,
        ListVal,
        FunctionVal,
        ModuleVal,
        ClassVal,
        InstanceVal,
        EnumVal,
        EnumVarVal,
        NativeFunctionVal
    };

    struct Value {
        Symbol* type {nullptr};
        bool important{false};

        virtual ValueType kind()=0;
        virtual std::shared_ptr<Value> copy()=0;
        [[nodiscard]] virtual bool is_numeric() const { return false; }
        [[nodiscard]] virtual bool is_copyable() const { return false; }

        [[nodiscard]] std::string address_as_str() const {
            std::stringstream ss;
            ss << this;
            return ss.str();
        }

        virtual std::string to_string() { return VALUE_AT_MSG + address_as_str(); }

        template<typename T>
        static std::shared_ptr<T> as(const std::shared_ptr<Value>& v) {
            return std::dynamic_pointer_cast<T>(v);
        }

        explicit Value(Symbol* sym): type(sym) { }
    };

    struct NormalValue final: public Value {
        std::any val;

        ValueType kind() final { return ValueType::NormalVal; }
        [[nodiscard]] bool is_numeric() const final { return type->name == INT_TP || type->name == DOUBLE_TP; }
        [[nodiscard]] bool is_copyable() const final { return type->name != NULL_TP; }

        std::shared_ptr<Value> copy() final;

        int as_int();
        double as_double();
        bool as_bool();
        std::string as_string();

        std::string to_string() final;
        NormalValue(Symbol *tp, std::any the_value);

        static std::shared_ptr<NormalValue> create(Symbol *tp, std::any the_value);
    };

    struct ListValue final: public Value {
        std::vector<Symbol> elements;
        ValueType kind() final { return ValueType::ListVal; }
        [[nodiscard]] bool is_copyable() const final { return true; }

        std::shared_ptr<Value> copy() final;

        std::string to_string() final;
        std::vector<std::shared_ptr<Value>> as_list_value();
        ListValue(Symbol* tp, std::vector<Symbol> sym_elements);

        static std::shared_ptr<ListValue> create(Symbol* tp, std::vector<Symbol> sym_elements);
    };

    struct FunctionValue final: public Value {
        std::vector<std::shared_ptr<Parsing::Node>> params;
        std::shared_ptr<Parsing::Node> body;
        SymbolTable* parentScope{};
        std::string name;
        ValueType kind() final { return ValueType::FunctionVal; }

        std::shared_ptr<Value> copy() final;

        std::string to_string() final { return FUNC_AT_MSG + address_as_str(); }

        FunctionValue(Symbol* tp, std::vector<std::shared_ptr<Parsing::Node>> params_, std::shared_ptr<Parsing::Node> body_, SymbolTable* scope_, std::string name="<anonymous>");

        static std::shared_ptr<FunctionValue> create(Symbol* tp, std::vector<std::shared_ptr<Parsing::Node>> params_, std::shared_ptr<Parsing::Node> body_, SymbolTable* scope_, std::string name="<anonymous>");
    };

    struct ModuleValue final: public Value {
        SymbolTable ownScope;
        ValueType kind() final { return ValueType::ModuleVal; }

        std::shared_ptr<Value> copy() final;

        std::string to_string() final { return MODULE_AT_MSG + address_as_str(); }

        ModuleValue(Symbol* tp, const SymbolTable& scope);

        static std::shared_ptr<ModuleValue> create(Symbol* tp, const SymbolTable& scope);
    };

    struct ClassValue final: public Value {
        SymbolTable ownScope;
        std::shared_ptr<Parsing::Node> body;
        // What's the point of the parent scope if ownscope has it?
        SymbolTable* parentScope;
        ValueType kind() final { return ValueType::ClassVal; }

        std::shared_ptr<Value> copy() final;

        std::string to_string() final { return CLASS_AT_MSG + address_as_str(); }

        ClassValue(Symbol* tp, const SymbolTable& scope, SymbolTable* parent_, std::shared_ptr<Parsing::Node> body_);

        static std::shared_ptr<ClassValue> create(Symbol* tp, const SymbolTable& scope, SymbolTable* parent_, std::shared_ptr<Parsing::Node> body_);

        Symbol* getStaticVarSymbol(const std::string& name);
    };

    struct InstanceValue final: public Value {
        std::shared_ptr<ClassValue> molde;
        SymbolTable ownScope;

        ValueType kind() final { return ValueType::InstanceVal; }

        std::shared_ptr<Value> copy() final;

        std::string to_string() final { return INSTANCE_AT_MSG + address_as_str(); }

        InstanceValue(Symbol* tp, std::shared_ptr<ClassValue> molde_, const SymbolTable& scope);

        static std::shared_ptr<InstanceValue> create(Symbol* tp, std::shared_ptr<ClassValue> molde_, const SymbolTable& scope);

        Symbol* getStaticVarSymbol(const std::string& name);
    };

    struct EnumValue final: public Value {
        SymbolTable ownScope;
        ValueType kind() final { return ValueType::EnumVal; }

        std::shared_ptr<Value> copy() final;

        std::string to_string() final { return ENUM_AT_MSG + address_as_str(); }

        EnumValue(Symbol* tp, const SymbolTable& scope);

        static std::shared_ptr<EnumValue> create(Symbol* tp, const SymbolTable& scope);
    };

    struct EnumVarValue final: public Value {
        std::string name;
        ValueType kind() final { return ValueType::EnumVarVal; }

        std::shared_ptr<Value> copy() final;

        std::string to_string() final { return name; }

        EnumVarValue(Symbol* tp, std::string name_);

        static std::shared_ptr<EnumVarValue> create(Symbol* tp, std::string name_);
    };

    class Interpreter;
    typedef std::function<std::shared_ptr<Value>(Interpreter&, std::vector<std::shared_ptr<Value>>)> native_function;
    struct NativeFunctionValue final: public Value {
        native_function fn;
        ValueType kind() final { return ValueType::NativeFunctionVal; }

        std::shared_ptr<Value> copy() final;

        std::shared_ptr<Value> visit(Interpreter&, std::vector<std::shared_ptr<Value>>) const;

        NativeFunctionValue(Symbol*, native_function);

        static std::shared_ptr<NativeFunctionValue> create(Symbol*, native_function);
    };

}
#endif //ODO_PORT_VALUE_H
