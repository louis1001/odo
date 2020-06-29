//
// Created by Luis Gonzalez on 3/20/20.
//

#include "Interpreter/Interpreter.h"
#include "Exceptions/exception.h"
#include "IO/io.h"
#include "utils.h"

#include <cmath>
#include <iostream>
#include <utility>

#define noop (void)0
// #define DEBUG_FUNCTIONS

namespace Odo::Interpreting {
    using namespace Parsing;

    Interpreter::Interpreter(Parser p): parser(std::move(p)) {
        auto any_symbol = Symbol{.name="any", .isType=true, .kind=PrimitiveType};

        std::map<std::string, Symbol> buildInTypes = {
            {"any", any_symbol}
        };
        globalTable = SymbolTable("global", buildInTypes);

        auto any_sym = &globalTable.symbols["any"];

        globalTable.symbols["int"] = {.tp=any_sym, .name="int", .isType=true, .kind=PrimitiveType};
        globalTable.symbols["double"] = Symbol{.tp=any_sym, .name="double", .isType=true, .kind=PrimitiveType};
        globalTable.symbols["string"] = Symbol{.tp=any_sym, .name="string", .isType=true, .kind=PrimitiveType};
        globalTable.symbols["bool"] = Symbol{.tp=any_sym, .name="bool", .isType=true, .kind=PrimitiveType};
        globalTable.symbols["pointer"] = Symbol{.tp=any_sym, .name="pointer", .isType=true, .kind=PrimitiveType};
        globalTable.symbols["NullType"] = Symbol{.tp=any_sym, .name="NullType", .isType=true, .kind=PrimitiveType};

        currentScope = &globalTable;

        replScope = SymbolTable("repl", {}, &globalTable);

        valueTable = ValueTable();

        null = valueTable.addNewValue(
            &globalTable.symbols["NullType"],
            "null"
        );

        auto nullSym = globalTable.addSymbol({
            .tp = &globalTable.symbols["NullType"],
            .name = "null",
            .value = null
        });
        null->addReference(*nullSym);

        returning = nullptr;

#ifdef DEBUG_FUNCTIONS
        add_native_function("valueAt", [&](auto values) {
            int a = values[0]->as_int();

            return &valueTable.value_map()[a];
        });
#endif

        add_native_function("print", [&](auto values) {
            for (auto v : values) {
                if (v)
                    std::cout << v->to_string();
            }
            // Might make printing slower... I don't know of a better way of doing this.
            std::cout.flush();
            return null;
        });

        add_native_function("println", [&](auto values) {
            for (auto v : values) {
                if (v)
                    std::cout << v->to_string();
            }
            std::cout << std::endl;
            return null;
        });

        add_native_function("move_cursor", [&](auto vals) {
            std::cout << "\033[" << vals[0]->as_int() << ";" << vals[1]->as_int() << "H";

            return null;
        });

        add_native_function("factorial", [&](auto v){
            if (!v.empty() && v[0]->type->name == "int") {
                int arg1 = v[0]->as_int();
                int result = 1;
                for(int i = 1; i <= arg1; i++) {
                    result = result * i;
                }
                return create_literal(result);
            }
            throw Exceptions::FunctionCallException(
                "factorial function requires a single int argument.",
                current_line,
                current_col
            );
            return null;
        });

        add_native_function("length", [&](std::vector<Value*> v){
            if (!v.empty()) {
                auto arg = v[0];
                if (arg->type->name == "string") {
                    size_t len = arg->as_string().size();
                    return create_literal((int)len);
                } else if (arg->kind == ListVal) {
                    size_t len = arg->as_list_value().size();
                    return create_literal((int)len);
                }
            }
            throw Exceptions::FunctionCallException(
                "length function requires a single argument of type string or list.",
                current_line,
                current_col
            );
            return null;
        });

        add_native_function("pow", [&](auto vals) {
            if (vals.size() >= 2) {
                bool result_as_int = true;
                double a = 0, b = 0;

                if (vals[0]->type->name == "double") {
                    a = vals[0]->as_double();
                    result_as_int = false;
                } else if (vals[0]->type->name == "int") {
                    a = vals[0]->as_int();
                }

                if (vals[1]->type->name == "double") {
                    b = vals[1]->as_double();
                    result_as_int = false;
                } else if (vals[1]->type->name == "int") {
                    b = vals[1]->as_int();
                }

                if (result_as_int) {
                    return create_literal((int) trunc(pow(a, b)));
                } else {
                    return create_literal(pow(a, b));
                }
            }
            return null;
        });

        add_native_function("sqrt", [&](auto vals) {
            if (!vals.empty()) {
                double a = 0;

                if (vals[0]->type->name == "double") {
                    a = vals[0]->as_double();
                } else if (vals[0]->type->name == "int") {
                    a = vals[0]->as_int();
                }

                return create_literal(sqrt(a));
            }
            return null;
        });
        add_native_function("sin", [&](auto vals) {
            if (!vals.empty()) {
                double a = 0;

                if (vals[0]->type->name == "double") {
                    a = vals[0]->as_double();
                } else if (vals[0]->type->name == "int") {
                    a = vals[0]->as_int();
                }

                return create_literal(sin(a));
            }
            return null;
        });
        add_native_function("cos", [&](auto vals) {
            if (!vals.empty()) {
                double a = 0;

                if (vals[0]->type->name == "double") {
                    a = vals[0]->as_double();
                } else if (vals[0]->type->name == "int") {
                    a = vals[0]->as_int();
                }

                return create_literal(cos(a));
            }
            return null;
        });

        add_native_function("floor", [&](auto vals) {
            if (vals.size() == 1) {
                auto v1 = vals[0];
                double v = 0;
                if (v1->type->name == "double") {
                    v = v1->as_double();
                } else if (v1->type->name == "int") {
                    v = v1->as_int();
                } else {
                    throw Exceptions::ValueException("floor function can only be called with numeric values");
                }

                return create_literal(static_cast<int>(floor(v)));
            }

            return null;
        });

        add_native_function("trunc", [&](auto vals) {
            if (vals.size() == 1) {
                auto v1 = vals[0];
                double v = 0;
                if (v1->type->name == "double") {
                    v = v1->as_double();
                } else if (v1->type->name == "int") {
                    v = v1->as_int();
                } else {
                    throw Exceptions::ValueException("trunc function can only be called with numeric values");
                }

                return create_literal(static_cast<int>(trunc(v)));
            }

            return null;
        });

        add_native_function("round", [&](auto vals) {
            if (!vals.empty()) {
                auto v1 = vals[0];
                double v = 0;
                if (v1->type->name == "double") {
                    v = v1->as_double();
                } else if (v1->type->name == "int") {
                    v = v1->as_int();
                } else {
                    throw Exceptions::ValueException(
                        "trunc function can only be called with numeric values and an optional int+",
                        current_line,
                        current_col
                    );
                }

                if (vals.size() == 1) {
                    return create_literal(static_cast<int>(round(v)));
                }

                auto v2 = vals[1];
                if (v2->type->name != "int") {
                    throw Exceptions::ValueException(
                        "trunc function can only be called with a numeric value and an optional int-",
                        current_line,
                        current_col
                    );
                }

                int rounding = v2->as_int();
                int decimal = pow(10, rounding);
                double result = round(v*decimal)/decimal;
                return create_literal(result);
            }

            return null;
        });

        add_native_function("read", [&](const std::vector<Value*>& vals) {
            std::string result;
            for (auto v : vals) {
                std::cout << v->to_string();
            }

            std::getline(std::cin, result);
            return create_literal(result);
        });

        add_native_function("read_int", [&](const std::vector<Value*>& vals) {
            int result;
            for (auto v : vals) {
                std::cout << v->to_string();
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(result);
        });

        add_native_function("read_double", [&](const std::vector<Value*>& vals) {
            double result;
            for (auto v : vals) {
                std::cout << v->to_string();
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(result);
        });

        add_native_function("rand", [&](auto vals) {
            double min = 0.0;
            double max = 1.0;
            if (vals.size() == 1) {
                if (vals[0]->type->name == "double") {
                    max = vals[0]->as_double();
                } else if (vals[0]->type->name == "int") {
                    max = vals[0]->as_int();
                }
            } else if (vals.size() >= 2) {
                if (vals[0]->type->name == "double") {
                    min = vals[0]->as_double();
                } else if (vals[0]->type->name == "int") {
                    min = vals[0]->as_int();
                }

                if (vals[1]->type->name == "double") {
                    max = vals[1]->as_double();
                } else if (vals[1]->type->name == "int") {
                    max = vals[1]->as_int();
                }
            }

            return create_literal(rand_double(min, max));
        });

        add_native_function("randInt", [&](auto vals) {
            int min = 0;
            int max = INT32_MAX;
            if (vals.size() == 1) {
                max = vals[0]->as_int();
            } else if (vals.size() >= 2) {
                min = vals[0]->as_int();
                max = vals[1]->as_int();
            }

            return create_literal(rand_int(min, max));
        });

        add_native_function("pop", [&](auto vals) {
            if (vals.size() == 1) {
                auto& lst = vals[0];
                if (lst->kind == ListVal) {
                    std::vector<Symbol>& syms = lst->as_list_symbol();

                    if (!syms.empty()) {
                        auto* v = &*syms.end()-1;
                        syms.pop_back();

                        return (v && v->value) ? v->value : null;
                    }
                }
            }

            return null;
        });

        add_native_function("typeof", [&](const auto& vals) {
            if (vals.size() == 1) {
                auto v = vals[0];
                if (v) return create_literal(v->type->name);
            }

            return null;
        });

        add_native_function("clear", [&](auto){std::cout << "\033[2J\033[1;1H"; return null;});

        add_native_function("wait", [&](auto){ std::cin.get(); return null; });
    }

    Symbol* Interpreter::any_type() {
        return &globalTable.symbols["any"];
    }

    std::pair<Value*, Value*> Interpreter::coerce_type(Value* lhs, Value*rhs) {
        std::pair<Value*, Value*> result = {lhs, rhs};
        if (lhs->type->kind != PrimitiveType || rhs->type->kind != PrimitiveType)
            return result;

        auto is_numerical = [](Value* v) {
            return v->type->name == "int" || v->type->name == "double";
        };

        // Check if both values are numerical
        if (lhs->type->name != rhs->type->name && is_numerical(lhs) && is_numerical(rhs)) {
            if (lhs->type->name == "int") {
                auto new_left = create_literal((double)std::any_cast<int>(lhs->val));

                result.first = new_left;
            }

            if (rhs->type->name == "int") {
                auto new_left = create_literal((double)std::any_cast<int>(rhs->val));

                result.second = new_left;
            }
        }

        return result;
    }

    Value* Interpreter::visit(AST node) {
        current_line = node.line_number;
        current_col = node.column_number;
        switch (node.tp) {
            // Normal Types
            case Double:
                return visit_Double(node.token);
            case Int:
                return visit_Int(node.token);
            case Bool:
                return visit_Bool(node.token);
            case Str:
                return visit_Str(node.token);

            // Operations
            case BinOp:
                return visit_BinOp(node.token, node.nodes["left"], node.nodes["right"]);
                break;
            case UnaryOp:
                return visit_UnaryOp(node.token, node.nodes["right"]);
                break;
            case NoOp:
                return null;

            case Index:
                return visit_Index(node.nodes["val"], node.nodes["expr"]);
                break;

            // Control Flow
            case TernaryOp:
                return visit_TernaryOp(node.nodes["cond"], node.nodes["trueb"], node.nodes["falseb"]);
                break;
            case If:
                return visit_If(node.nodes["expr"], node.nodes["trueb"], node.nodes["falseb"]);
                break;
            case For:
                return visit_For(node.nodes["ini"], node.nodes["cond"], node.nodes["incr"], node.nodes["body"]);
                break;
            case ForEach:
                return visit_ForEach(node.token, node.nodes["lst"], node.nodes["body"]);
                break;
            case While:
                return visit_While(node.nodes["cond"], node.nodes["body"]);
                break;
            case Loop:
                return visit_Loop(node.nodes["body"]);
                break;
            case Break:
                breaking = true;
                return null;
            case Continue:
                continuing = true;
                return null;
            case Block:
                return visit_Block(node.lst_AST);

            // Variable Handling
            case Declaration:
                return visit_VarDeclaration(node.type, node.token, node.nodes["initial"]);
            case ListDeclaration:
                return visit_ListDeclaration(node.type, node.token, node.nodes["initial"]);
                break;
            case Variable:
                return visit_Variable(node.token);
                break;
            case Assignment:
                return visit_Assignment(node.nodes["token"], node.nodes["right"]);
                break;

            case ListExpression:
                return visit_ListExpression(node.lst_AST);
                break;

            // Functions
            case FuncExpression:
                return visit_FuncExpression(node.lst_AST, node.type, node.nodes["body"]);
                break;
            case FuncDecl:
                return visit_FuncDecl(node.token, node.lst_AST, node.type, node.nodes["body"]);
                break;
            case FuncCall:
                return visit_FuncCall(node.nodes["fun"], node.token, node.lst_AST);
                break;
            case FuncBody:
                return visit_FuncBody(node.lst_AST);
                break;
            case Return:
                return visit_Return(node.nodes["val"]);
                break;

            case Enum:
                return visit_Enum(node.token, node.lst_AST);

            // Classes
            case Class:
                return visit_Class(node.token, node.type, node.nodes["body"]);
                break;
            case ClassBody:
                return visit_ClassBody(node.lst_AST);
                break;
//          Broken or incomplete.
            case ConstructorDecl:
                return visit_ConstructorDecl(node.lst_AST, node.nodes["body"]);
            case ConstructorCall:
                return visit_ConstructorCall(node.token);
            case InstanceBody:
                return visit_InstanceBody(node.lst_AST);
            case ClassInitializer:
                return visit_ClassInitializer(node.token, node.lst_AST);
            case StaticStatement:
                throw Exceptions::OdoException(
                    "Static statements can only appear inside a class body.",
                    current_line,
                    current_col
                );
            case MemberVar:
                return visit_MemberVar(node.nodes["inst"], node.token);
                break;
            case StaticVar:
                return visit_StaticVar(node.nodes["inst"], node.token);
                break;

            case Module:
                return this->visit_Module(node.token, node.lst_AST);
                break;
            case Import:
                return this->visit_Import(node.token, node.type);
                break;

            case Debug:
                noop;
            case Null:
                return null;
        }
        return null;
    }

    int Interpreter::add_native_function(const std::string& name, NativeFunction callback) {
        auto result = native_functions.find(name);
        if (result != native_functions.end())
            return -1;

        native_functions[name] = std::move(callback);
        return 0;
    }

    Value* Interpreter::create_literal_from_string(std::string val, const std::string& kind) {
        std::any newValue;
        if (kind == "double") {
            newValue = strtod(val.c_str(), nullptr);
        } else if (kind == "int") {
            int a = (int) strtol(val.c_str(), nullptr, 10);
            newValue = a;
        } else if (kind == "string") {
            newValue = val;
        } else if (kind == "bool") {
            if (val != "true" && val != "false"){
                throw Exceptions::ValueException(
                    "Invalid value for bool expression.",
                    current_line,
                    current_col
                );
            }
            newValue = val == "true";
        } else {
            return null;
        }

        // Handle errors in conversions are incorrect.

        auto the_value = valueTable.addNewValue(
            globalTable.findSymbol(kind),
            newValue
        );

        return the_value;
    }

    Value* Interpreter::create_literal_from_any(std::any val, const std::string &kind) {
        return valueTable.addNewValue(
            globalTable.findSymbol(kind),
            std::move(val)
        );
    }

    Value* Interpreter::create_literal(std::string val) {
        return create_literal_from_any(val, "string");
    }

    Value* Interpreter::create_literal(int val) {
        return create_literal_from_any(val, "int");
    }

    Value* Interpreter::create_literal(double val) {
        return create_literal_from_any(val, "double");
    }

    Value* Interpreter::create_literal(bool val) {
        return create_literal_from_any(val, "bool");
    }

    Value* Interpreter::visit_Double(const Lexing::Token& t) {
        return create_literal_from_string(t.value, "double");
    }

    Value* Interpreter::visit_Int(const Lexing::Token& t) {
        return create_literal_from_string(t.value, "int");
    }

    Value* Interpreter::visit_Bool(const Lexing::Token& t) {
        return create_literal_from_string(t.value, "bool");
    }

    Value* Interpreter::visit_Str(const Lexing::Token& t) {
        return create_literal_from_string(t.value, "string");
    }

    Value* Interpreter::visit_Block(const std::vector<AST>& statements) {
        auto blockScope = SymbolTable("block_scope", {}, currentScope);
        currentScope = &blockScope;

        auto result = null;
        for (const auto& st : statements) {
            result = visit(st);
            if (breaking || continuing || returning) {
                break;
            }
        }

        result->important = true;
        currentScope = blockScope.getParent();
        valueTable.cleanUp(blockScope);
        result->important = false;

        return result;
    }

    Value* Interpreter::visit_TernaryOp(const AST& cond, AST trueb, AST falseb) {
        auto val_cond = visit(cond);
        if (val_cond->type->name != "bool") {
            throw Exceptions::TypeException(
                    "Condition of ternary expression must be boolean.",
                    cond.line_number,
                    cond.column_number
            );
        }
        bool real_condition = std::any_cast<bool>(val_cond->val);

        if (real_condition) {
            return visit(std::move(trueb));
        } else {
            return visit(std::move(falseb));
        }
        return nullptr;
    }

    Value* Interpreter::visit_If(const AST& cond, AST trueb, AST falseb) {
        auto val_cond = visit(cond);
        if (val_cond->type->name != "bool") {
            throw Exceptions::TypeException(
                    "Condition of if statement must be boolean.",
                    cond.line_number,
                    cond.column_number
            );
        }
        bool real_condition = std::any_cast<bool>(val_cond->val);

        if (real_condition) {
            return visit(std::move(trueb));
        } else {
            return visit(std::move(falseb));
        }
        return nullptr;
    }

    Value* Interpreter::visit_For(AST ini, const AST& cond, const AST& incr, const AST& body) {
        auto forScope = SymbolTable("for:loop", {}, currentScope);
        currentScope = &forScope;

        visit(std::move(ini));

        auto val_cond = visit(cond);
        if (val_cond->type->name != "bool") {
            throw Exceptions::TypeException(
                    "Condition expression of for statement must be boolean.",
                    cond.line_number,
                    cond.column_number
            );
        }

        auto actual_cond = val_cond->as_bool();
        while(actual_cond){
            if (continuing) {
                continuing = false;
                continue;
            } else {
                visit(body);
            }

            if (breaking) {
                breaking = false;
                break;
            }

            if (returning) {
                break;
            }

            visit(incr);

            actual_cond = visit(cond)->as_bool();
        }

        continuing = false;

        currentScope = forScope.getParent();
        valueTable.cleanUp(forScope);

        return null;
    }

    Value* Interpreter::visit_ForEach(const Lexing::Token& v, const Parsing::AST& lst, const Parsing::AST& body) {
        auto forScope = SymbolTable("foreach:loop", {}, currentScope);
        currentScope = &forScope;

        auto lst_value = visit(lst);
        if (lst_value->kind == ListVal) {
            AST iterator_decl;
            if (lst_value->type->tp && lst_value->type->tp->kind == ListType) {
                iterator_decl.tp = ListDeclaration;
            } else {
                iterator_decl.tp = Declaration;
            }

            iterator_decl.type = Lexing::Token(Lexing::TokenType::ID, lst_value->type->name);
            iterator_decl.token = v;
            visit(iterator_decl);

            auto declared_iter = currentScope->findSymbol(v.value);

            for(auto& s: lst_value->as_list_symbol()){
                declared_iter->value = s.value;

                visit(body);
                if (continuing) {
                    continuing = false;
                    continue;
                }

                if (breaking) {
                    breaking = false;
                    break;
                }

                if (returning) {
                    break;
                }
            }
        } else if (lst_value->type->name == "string") {
            AST iterator_decl {Declaration};
            iterator_decl.type = Lexing::Token(Lexing::TokenType::ID, "string");
            iterator_decl.token = v;
            visit(iterator_decl);

            auto declared_iter = currentScope->findSymbol(v.value);

            for(auto& s: lst_value->as_string()){
                declared_iter->value = create_literal(std::string(1, s));

                visit(body);
                if (continuing) {
                    continuing = false;
                    continue;
                }

                if (breaking) {
                    breaking = false;
                    break;
                }

                if (returning) {
                    break;
                }
            }
        } else {
            throw Exceptions::ValueException(
                    "foreach statement can only be used with list or string values.",
                    current_line,
                    current_col
            );
        }

        currentScope = forScope.getParent();
        valueTable.cleanUp(forScope);

        return null;

    }

    Value *Interpreter::visit_While(const AST& cond, const AST& body) {
        auto whileScope = SymbolTable("while:loop", {}, currentScope);
        currentScope = &whileScope;

        auto val_cond = visit(cond);
        if (val_cond->type->name != "bool") {
            throw Exceptions::TypeException(
                    "Condition expression of for statement must be boolean.",
                    cond.line_number,
                    cond.column_number
            );
        }

        auto actual_cond = val_cond->as_bool();
        while(actual_cond){
            visit(body);
            if (breaking) {
                breaking = false;
                break;
            }
            if (continuing) {
                continuing = false;
                continue;
            }
            if (returning) {
                break;
            }

            actual_cond = visit(cond)->as_bool();
        }

        currentScope = whileScope.getParent();
        valueTable.cleanUp(whileScope);

        return null;
    }

    Value *Interpreter::visit_Loop(const AST& body) {
        while(true){
            visit(body);
            if (breaking) {
                breaking = false;
                break;
            }
            if (continuing) {
                continuing = false;
                continue;
            }
            if (returning) {
                break;
            }
        }

        return null;
    }

    Value* Interpreter::visit_VarDeclaration(const Lexing::Token& var_type, const Lexing::Token& name, const AST& initial) {
        if (currentScope->symbolExists(name.value)) {
            throw Exceptions::NameException(
                    "Variable called '" + name.value + "' already exists",
                    current_line,
                    current_col
            );
        } else {
            auto type_ = currentScope->findSymbol(var_type.value);

            if (type_ == nullptr) {
                throw Exceptions::NameException(
                        "Unknown type '" + var_type.value + "'.",
                        current_line,
                        current_col
                );
            }

            Symbol newVar;
            Value* valueReturn;

            if (initial.tp != NoOp) {
                auto newValue = visit(initial);

                if (is_copyable(newValue)) {
                    newValue = valueTable.copyValue(*newValue);
                }

                if (type_->name == "any") {
                    type_ = newValue->type;
                } else {
                    if (type_->name == "int" && newValue->type->name == "double") {
                        newValue = create_literal((int) newValue->as_double());
                    } else if (type_->name == "double" && newValue->type->name == "int") {
                        newValue = create_literal((double) newValue->as_int());
                    }
                }

                newVar = {
                    type_,
                    name.value,
                    newValue
                };

                valueReturn = newValue;
            } else {
                newVar = {
                    type_,
                    name.value
                };

                valueReturn = null;
            }

            auto in_sym_table = currentScope->addSymbol(newVar);
            if (valueReturn != null) {
                valueReturn->addReference(*in_sym_table);
            }
            return valueReturn;
        }
        return null;
    }

    Value *Interpreter::visit_ListDeclaration(const Lexing::Token &var_type, const Lexing::Token& name, const AST& initial) {
        if (currentScope->symbolExists(name.value)) {
            throw Exceptions::NameException(
                    "Variable called '" + name.value + "' already exists",
                    current_line,
                    current_col
            );
        }

        auto base_type = currentScope->findSymbol(var_type.value);
        if (!(base_type && base_type->isType)) {
            throw Exceptions::TypeException(
                    "Invalid type '" + var_type.value + "'.",
                    current_line,
                    current_col
            );
        }

        Symbol* newVar;
        Value* valueReturn = null;

        Symbol* list_type;

        auto found_in_table = globalTable.findSymbol(base_type->name + "[]");
        if (found_in_table) {
            list_type = found_in_table;
        } else {
            list_type = globalTable.addSymbol({
                .tp=base_type,
                .name=base_type->name + "[]",
                .isType=true,
                .kind=ListType
            });
        }

        if (initial.tp != NoOp) {
            auto newValue = visit(initial);

            newVar = currentScope->addSymbol({
                list_type,
                name.value,
                newValue
            });

            newValue->addReference(*newVar);
            newValue->kind = ListVal;
            valueReturn = newValue;

            return valueReturn;
        }

        currentScope->addSymbol({
            list_type,
            name.value
        });

        return valueReturn;
    }

    Value* Interpreter::visit_Assignment(AST expr, AST val) {
        auto varSym = getMemberVarSymbol(std::move(expr));
        auto newValue = visit(std::move(val));

        if (varSym) {
            if (varSym->value) {
                auto theValue = varSym->value;

                theValue->removeReference(*varSym);
                if (is_copyable(newValue)) {
                    newValue = valueTable.addNewValue(newValue->type, newValue->val);
                }
            } else {
                if (is_copyable(newValue)) {
                    newValue = valueTable.addNewValue(newValue->type, newValue->val);
                }

                if (varSym->tp->name == "any") {
                    varSym->tp = newValue->type;
                } else {
                    if (varSym->tp->name == "int" && newValue->type->name == "double") {
                        newValue = create_literal((int) newValue->as_double());
                    } else if (varSym->tp->name == "double" && newValue->type->name == "int") {
                        newValue = create_literal((double) newValue->as_int());
                    }
                }
            }

            varSym->value = newValue;
            newValue->addReference(*varSym);
        } else {
            throw Exceptions::NameException(
                    "Assignment to unknwon variable.",
                    current_line,
                    current_col
            );
        }
        return null;
    }

    Value* Interpreter::visit_Variable(const Lexing::Token& token) {
        auto found = currentScope->findSymbol(token.value);

        if (found != nullptr) {
            if (found->value) {
                auto val = found->value;

                return (val == nullptr) ? null : val;
            } else {
                return null;
            }
        } else {
            throw Exceptions::NameException(
                    "Variable named '" + token.value + "' not defined.",
                    current_line,
                    current_col
            );
        }
    }

    Value* Interpreter::visit_Index(AST val, const AST& expr) {
        auto visited_val = visit(std::move(val));

        if (visited_val->type->name == "string") {
            auto str = visited_val->as_string();

            auto visited_indx = visit(expr);
            if (visited_indx->type->name == "int") {
                auto int_indx = visited_indx->as_int();

                if (int_indx >= 0 && int_indx < str.size()) {
                    std::string result(1, str[int_indx]);
                    return create_literal_from_string(result, "string");
                } else if (int_indx < 0 && abs(int_indx) <= str.size()) {
                    size_t actual_indx = str.size() - int_indx;
                    std::string result(1, str[actual_indx]);
                    return create_literal_from_string(result, "string");
                } else {
                    throw Exceptions::ValueException(
                            "Indexing a string out of bounds.",
                            current_line,
                            current_col
                    );
                }
            } else {
                throw Exceptions::TypeException(
                        "Strings can only be indexed with integer values.",
                        current_line,
                        current_col
                );
            }
        } else if (visited_val->kind == ListVal){
            auto list_value = visited_val->as_list_value();
            auto visited_indx = visit(expr);
            if (visited_indx->type->name == "int") {
                auto int_indx = visited_indx->as_int();

                if (int_indx >= 0 && int_indx < list_value.size()) {
                    return list_value[int_indx];
                } else if (int_indx < 0 && abs(int_indx) <= list_value.size()) {
                    size_t actual_indx = list_value.size() - int_indx;
                    return list_value[actual_indx];
                } else {
                    throw Exceptions::ValueException(
                            "Indexing a list out of bounds.",
                            current_line,
                            current_col
                    );
                }
            } else {
                throw Exceptions::TypeException(
                        "Lists can only be indexed with integer values.",
                        current_line,
                        current_col
                );
            }
        } else {
            throw Exceptions::ValueException(
                    "Index operator is only valid for strings and lists values.",
                    current_line,
                    current_col
            );
        }
    }

    Value *Interpreter::visit_ListExpression(const std::vector<AST>& elements) {
        Symbol* list_t = nullptr;
        std::vector<Symbol> list_syms;

        for (const auto& el : elements) {
            auto visited_element = visit(el);
            auto type_of_el = visited_element->type;

            if (!list_t) {
                auto list_type_name = visited_element->type->name + "[]";

                // TODO: Fix. Apparently list types are stored only in the global scope.
                auto found_type = globalTable.findSymbol(list_type_name);
                if (found_type) {
                    list_t = found_type;
                } else {
                    list_t = globalTable.addListType(type_of_el);
                }
            }

            Value* actual_value;

            if (is_copyable(visited_element)) {
                actual_value = valueTable.copyValue(*visited_element);
            } else {
                actual_value = visited_element;
            }

            auto el_symbol = Symbol{
                type_of_el,
                "list_element",
                actual_value
            };

            actual_value->addReference(el_symbol);
            list_syms.push_back(el_symbol);
        }

        Symbol* list_type;
        if (list_t) {
            list_type = list_t;
        } else {
            list_type = globalTable.addListType(any_type());
        }

        auto in_val_table = valueTable.addNewValue(list_type, list_syms);
        in_val_table->kind = ListVal;
        return in_val_table;
    }

    Value* Interpreter::visit_BinOp(const Lexing::Token& token, AST &left, AST &right) {
        auto leftVisited = visit(left);
        leftVisited->important = true;
        auto rightVisited = visit(right);
        leftVisited->important = false;

        auto coerced = coerce_type(leftVisited, rightVisited);
        leftVisited = coerced.first;
        rightVisited = coerced.second;

        switch (token.tp) {
            case Lexing::PLUS:
                if (leftVisited->kind == ListVal) {
                    Value* new_list;

                    if (rightVisited->kind == ListVal) {
                        std::vector<Symbol> new_elements;

                        for (const auto& el : leftVisited->as_list_symbol()) {
                            auto val = el.value;

                            if (is_copyable(val)) {
                                val = valueTable.copyValue(*val);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }
                        for (const auto& el : rightVisited->as_list_symbol()) {
                            auto val = el.value;

                            if (is_copyable(val)) {
                                val = valueTable.copyValue(*val);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }

                        new_list = valueTable.addNewValue(leftVisited->type, new_elements);
                        new_list->kind = ListVal;
                        return new_list;
                    } else {
                        std::vector<Symbol> new_elements;

                        for (const auto& el : leftVisited->as_list_symbol()) {
                            auto val = el.value;

                            if (is_copyable(val)) {
                                val = valueTable.copyValue(*val);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }

                        auto val = rightVisited;
                        if (is_copyable(val)) {
                            val = valueTable.copyValue(*val);
                        }

                        Symbol new_symbol = {val->type, "list_element", val};
                        val->addReference(new_symbol);
                        new_elements.push_back(new_symbol);
                        new_list = valueTable.addNewValue(
                        {
                            .type=globalTable.addListType(val->type),
                            .val=new_elements,
                            .kind=ListVal
                        });
                        return new_list;
                    }
                } else if (leftVisited->type->name == "string") {
                    return create_literal(leftVisited->as_string() + rightVisited->to_string());
                } else if (rightVisited->type->name == "string") {
                    return create_literal(leftVisited->to_string() + rightVisited->as_string());
                } else if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() + rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() + rightVisited->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            "Addition operation can only be used with values of the same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::MINUS:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() - rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() - rightVisited->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            "Numeric substraction can only be used with values of the same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::MUL:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() * rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() * rightVisited->as_double();
                        return create_literal(result);
                    }
                } else if (leftVisited->kind == ListVal && rightVisited->type->name == "int") {
                    int right_as_int = rightVisited->as_int();
                    std::vector<Symbol> new_elements;

                    for (int i = 0; i < right_as_int; i++) {
                        for (const auto& el : leftVisited->as_list_symbol()) {
                            auto val = el.value;

                            if (is_copyable(val)) {
                                val = valueTable.copyValue(*val);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }
                    }

                    auto new_list = valueTable.addNewValue(leftVisited->type, new_elements);
                    new_list->kind = ListVal;
                    return new_list;
                } else if (leftVisited->type->name == "string" && rightVisited->type->name == "int"){
                    int right_as_int = rightVisited->as_int();
                    std::string left_as_string = leftVisited->as_string();
                    std::string new_string;

                    for (int i = 0; i < right_as_int; i++) {
                        new_string += left_as_string;
                    }

                    auto new_val = create_literal(new_string);
                    return new_val;
                } else {
                    throw Exceptions::TypeException(
                            "Multiplication operation can only be used with values of the same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::DIV:
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == "double") {
                    auto result = leftVisited->as_double() / rightVisited->as_double();
                    return create_literal(result);
                }else {
                    throw Exceptions::TypeException(
                            "Division operation can only be used with values of type double.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::MOD:
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == "int") {
                    auto result = leftVisited->as_int() % rightVisited->as_int();
                    return create_literal(result);
                }else {
                    throw Exceptions::TypeException(
                            "Modulo operation can only be used with values of type int.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::POW:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = powl(leftVisited->as_int(), rightVisited->as_int());
                        return create_literal((double)result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = powl(leftVisited->as_double(), rightVisited->as_double());
                        return create_literal((double)result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            "Power operation can only be used with values of the same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::EQU:
                if (leftVisited == rightVisited)
                    return create_literal(true);

                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() == rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() == rightVisited->as_double();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "bool") {
                        auto result = leftVisited->as_bool() == rightVisited->as_bool();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "string") {
                        auto result = leftVisited->as_string() == rightVisited->as_string();
                        return create_literal(result);
                    } else {
                        return create_literal(false);
                    }
                } else if (leftVisited->type == null->type || rightVisited->type == null->type) {
                    return create_literal(false);
                } else {
                    throw Exceptions::TypeException(
                            "Comparison operation can only be used with values of same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::NEQ:
                if (leftVisited == rightVisited)
                    return create_literal(false);

                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() != rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() != rightVisited->as_double();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "bool") {
                        auto result = leftVisited->as_bool() != rightVisited->as_bool();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "string") {
                        auto result = leftVisited->as_string() != rightVisited->as_string();
                        return create_literal(result);
                    } else {
                        return create_literal(true);
                    }
                } else if (leftVisited->type == null->type || rightVisited->type == null->type) {
                    return create_literal(true);
                } else {
                    throw Exceptions::TypeException(
                            "Comparison operation can only be used with values of same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::LT:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() < rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() < rightVisited->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            "Comparison operation can only be used with values of same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::GT:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() > rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() > rightVisited->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            "Comparison operation can only be used with values of same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::LET:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() <= rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() <= rightVisited->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            "Comparison operation can only be used with values of same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::GET:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = leftVisited->as_int() >= rightVisited->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = leftVisited->as_double() >= rightVisited->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            "Comparison operation can only be used with values of same type.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::AND:
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == "bool") {
                    auto result = leftVisited->as_bool() && rightVisited->as_bool();
                    return create_literal(result);
                }else {
                    throw Exceptions::TypeException(
                            "Logical operator can only be used with values of type bool.",
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::OR:
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == "bool") {
                    auto result = leftVisited->as_bool() || rightVisited->as_bool();
                    return create_literal(result);
                }else {
                    throw Exceptions::TypeException(
                            "Logical operator can only be used with values of type bool.",
                            current_line,
                            current_col
                    );
                }
                break;
            default:
                break;
        }

        return null;
    }

    Value* Interpreter::visit_UnaryOp(const Lexing::Token& token, AST &left) {
        auto result = visit(left);

        switch (token.tp) {
            case Lexing::PLUS:
                break;
            case Lexing::MINUS:
                if (result->type->name == "int") {
                    auto actual_value = result->as_int();

                    result->val = actual_value * -1;
                    return result;
                }

                if (result->type->name == "double") {
                    auto actual_value = result->as_double();

                    result->val = actual_value * -1;
                    return result;
                }

                throw Exceptions::TypeException(
                        "Unary operator can be used with int or double values.",
                        current_line,
                        current_col
                );
            default:
                break;
        }

        return null;
    }

    Value* Interpreter::visit_Module(const Lexing::Token& name, const std::vector<Parsing::AST>& statements) {
        SymbolTable module_scope = {
            "module-scope",
            {},
            currentScope
        };

        auto moduleValue = valueTable.addNewValue({
            .type=null->type,
            .val=0,
            .kind=ModuleVal,
            .ownScope=module_scope
        });
        moduleValue->important = true;

        auto temp = currentScope;
        currentScope = &moduleValue->ownScope;
        for (const auto& st : statements) {
            visit(st);
        }

        currentScope = temp;
        moduleValue->important = false;

        auto module_sym = currentScope->addSymbol({nullptr, name.value, moduleValue});
        moduleValue->addReference(*module_sym);

        return moduleValue;
    }

    Value * Interpreter::visit_Import(const Lexing::Token& path, const Lexing::Token& name) {
        interpret_as_module(path.value, name);
        return null;
    }

    Value* Interpreter::visit_FuncExpression(const std::vector<AST>& params, const Lexing::Token& retType, AST body){
        auto returnType = retType.tp == Lexing::NOTHING ? nullptr : currentScope->findSymbol(retType.value);

        auto paramTypes = getParamTypes(params);

        auto typeName = Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(returnType, paramTypes);
        }

        auto funcValue = valueTable.addNewValue(typeOfFunc, body);
        funcValue->scope = currentScope;
        funcValue->params = params;
        funcValue->kind = FunctionVal;

        return funcValue;
    }

    Value* Interpreter::visit_FuncDecl(const Lexing::Token& name, const std::vector<AST>& params, const Lexing::Token& retType, AST body){

        if (currentScope->symbolExists(name.value)) {
            throw Exceptions::NameException(
                    "Variable called '" + name.value + "' already exists",
                    current_line,
                    current_col
            );
        }

        auto returnType = retType.tp == Lexing::NOTHING ? nullptr : currentScope->findSymbol(retType.value);

        auto paramTypes = getParamTypes(params);

        auto typeName = Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(returnType, paramTypes);
        }

        auto funcValue = valueTable.addNewValue({
            .type=typeOfFunc,
            .val=body,
            .kind=FunctionVal,
            .scope=currentScope,
            .params=params
        });

        auto funcSymbol = currentScope->addSymbol({
            .tp=typeOfFunc,
            .name=name.value,
            .value=funcValue,
            .kind=FunctionSymbol
        });

        funcValue->addReference(*funcSymbol);

        return null;
    }

    Value *Interpreter::visit_FuncCall(AST expr, const Lexing::Token& fname, std::vector<AST> args) {
        if (call_stack.size() >= MAX_CALL_DEPTH) {
            throw Exceptions::RecursionException("Callback depth exceeded.", current_line, current_col);
        }

        if (fname.tp != Lexing::NOTHING){
            auto found_in_natives = native_functions.find(fname.value);

            if (found_in_natives != native_functions.end()) {
                std::vector<Value*> arguments_visited;
                arguments_visited.reserve(args.size());

                for(const auto& arg : args){
                    auto v = visit(arg);
                    v->important = true;
                    arguments_visited.push_back(v);
                }

                auto result = found_in_natives->second(arguments_visited);
                for (auto v : arguments_visited) v->important = false;

                return result;
            }
        }

        if (auto fVal = visit(std::move(expr));
            fVal->kind == FunctionVal) {

            auto funcScope = SymbolTable("func-scope", {}, fVal->scope);
            auto calleeScope = currentScope;

            std::vector<AST> newDecls;
            std::vector< std::pair<Lexing::Token, Value*> > initValues;

            for (int i = 0; i < fVal->params.size(); i++) {
                auto par = fVal->params[i];
                if (args.size() > i) {
                    switch (par.tp) {
                        case Declaration:
                        {
                            auto newValue = visit(args[i]);
                            if (is_copyable(newValue)) {
                                newValue = valueTable.copyValue(*newValue);
                            }
                            initValues.emplace_back(par.token, newValue);
                            break;
                        }
                        case ListDeclaration:
                        {
                            auto newValue = visit(args[i]);
                            initValues.emplace_back(par.token, newValue);
                            break;
                        }
                        default:
                            break;
                    }
                }

                newDecls.push_back(par);
            }

            currentScope = &funcScope;

            std::string function_name = "<anonimous>";
            if (!fVal->references.empty()) {
                auto& ref = *fVal->references.begin();
                function_name = ref.name;
            }

            call_stack.push_back({function_name, current_line, current_col});

            for (int i = 0; i < newDecls.size(); i++) {
                visit(newDecls[i]);

                if (i < initValues.size()) {
                    auto newVar = currentScope->findSymbol(initValues[i].first.value);
                    newVar->value = initValues[i].second;
                    initValues[i].second->addReference(*newVar);
                }
            }

            auto body_as_ast = std::any_cast<AST>(fVal->val);

            auto result = visit(body_as_ast);
            currentScope = calleeScope;
            valueTable.cleanUp(funcScope);

            result->important = false;
            call_stack.pop_back();
            return result;
        }

        throw Exceptions::ValueException("Value is not a function.", current_line, current_col);
    }

    Value *Interpreter::visit_FuncBody(const std::vector<AST>& statements) {
        auto temp = currentScope;
        auto bodyScope = SymbolTable("func-body-scope", {}, currentScope);

        currentScope = &bodyScope;

        for (const auto& st : statements) {
            visit(st);
            if (returning) {
                break;
            }
        }

        currentScope = temp;
        valueTable.cleanUp(bodyScope);

        auto ret = returning;
        returning = nullptr;
        return ret ? ret : null;
    }

    Value* Interpreter::visit_Return(AST val) {
        returning = visit(std::move(val));
        returning->important = true;
        return null;
    }

    Value* Interpreter::visit_Enum(const Lexing::Token& name, const std::vector<AST>& variants) {
        if (currentScope->symbolExists(name.value)) {
            throw Exceptions::NameException(
                    "Variable called '" + name.value + "' already exists",
                    current_line,
                    current_col
            );
        }

        Symbol newEnumSym = {
            .name=name.value,
            .isType=true,
            .kind=EnumType
        };

        Symbol* enumInTable = currentScope->addSymbol(newEnumSym);

        SymbolTable enum_variant_scope;

        for (int i = 0; i < variants.size(); i++) {
            auto variant = variants[i];
            auto variant_name = variant.token.value;

            auto variant_value = valueTable.addNewValue({
                .type=enumInTable,
                .val=variant_name,
                .kind=EnumVarVal
            });

            auto in_scope = enum_variant_scope.addSymbol({
                enumInTable,
                variant_name,
                variant_value
            });

            variant_value->addReference(*in_scope);
        }

        auto newValue = valueTable.addNewValue({
            .kind=EnumVal,
            .ownScope=enum_variant_scope
        });
        newValue->addReference(*enumInTable);
        enumInTable->value = newValue;

        return null;
    }

    Value* Interpreter::visit_Class(const Lexing::Token& name, const Lexing::Token& ty, AST body) {
        Symbol* typeSym = nullptr;

        if (ty.tp != Lexing::NOTHING) {
            auto sym = currentScope->findSymbol(ty.value);
            if (!sym || !sym->isType) {
                throw Exceptions::TypeException(
                        "Class must inherit from a type. " + name.value + " is invalid.",
                        current_line,
                        current_col
                );
            }

            typeSym = sym;
        }

        Symbol newClassSym = {
            .tp=typeSym,
            .name=name.value,
            .isType = true,
            .kind = ClassType,
        };
        auto inTable = currentScope->addSymbol(newClassSym);

        SymbolTable classScope{"class-" + name.value + "-scope", {}, currentScope};
        auto newClassMolde = valueTable.addNewValue({
            .type=inTable,
            .val=body,
            .kind=ClassVal,
            .scope=currentScope,
            .ownScope=classScope
        });

        auto prevScope = currentScope;
        currentScope = &newClassMolde->ownScope;

        visit(body);
        currentScope = prevScope;

        inTable->value = newClassMolde;
        newClassMolde->addReference(*inTable);

        return null;
    }

    Value* Interpreter::visit_ClassBody(std::vector<Parsing::AST> statements) {
        for (auto& st : statements) {
            if (st.tp == StaticStatement)
                visit(st.nodes["statement"]);
        }

        return null;
    }

    Value* Interpreter::visit_ConstructorDecl(const std::vector<Parsing::AST>& params, Parsing::AST body) {
        Symbol* retType = nullptr;

        auto paramTypes = getParamTypes(params);

        auto typeName = Symbol::constructFuncTypeName(retType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(retType, paramTypes);
        }

        auto funcValue = valueTable.addNewValue(typeOfFunc, body);
        funcValue->kind = FunctionVal;
        funcValue->scope = currentScope;
        funcValue->params = params;

        auto newFunctionSymbol = currentScope->addSymbol({typeOfFunc, "constructor", funcValue});
        funcValue->addReference(*newFunctionSymbol);

        return null;
    }

    Value* Interpreter::visit_ConstructorCall(const Lexing::Token& t) {
        if (call_stack.size() >= MAX_CALL_DEPTH) {
            throw Exceptions::RecursionException("Callback depth exceeded.", current_line, current_col);
        }

        auto constr = currentScope->findSymbol(t.value);

        auto fVal = constr ? constr->value : nullptr;

        // If there's no constructor, just ignore it.
        if (!fVal) {
            constructorParams.clear();
            constructorParams.shrink_to_fit();
            return null;
        }

        if (fVal->kind == FunctionVal) {
            SymbolTable funcScope = {"constructor-scope", {}, fVal->scope};
            auto calleeScope = currentScope;

            std::vector<AST> newDecls;
            std::vector< std::pair<Lexing::Token, Value*> > initValues;

            for (int i = 0; i < fVal->params.size(); i++) {
                auto par = fVal->params[i];
                if (constructorParams.size() > i) {
                    switch (par.tp) {
                        case Declaration:
                        case ListDeclaration:
                        {
                            auto newValue = constructorParams[i];
                            if (is_copyable(newValue)) {
                                newValue = valueTable.copyValue(*newValue);
                            }
                            initValues.emplace_back(par.token, newValue);
                            break;
                        }
                        default:
                            break;
                    }
                }

                newDecls.push_back(par);
            }

            currentScope = &funcScope;

            call_stack.push_back({"<constructor>", current_line, current_col});

            for (int i = 0; i < newDecls.size(); i++) {
                visit(newDecls[i]);

                if (i < initValues.size()) {
                    auto newVar = currentScope->findSymbol(initValues[i].first.value);
                    newVar->value = initValues[i].second;
                    initValues[i].second->addReference(*newVar);
                }
            }
            auto body_as_ast = std::any_cast<AST>(fVal->val);

            visit(body_as_ast);
            valueTable.cleanUp(funcScope);
            currentScope = calleeScope;
            call_stack.pop_back();
        } else {
            // Error.
            throw Exceptions::ValueException(
                    "Invalid constructor",
                current_line,
                current_col
            );
        }
        constructorParams.clear();
        constructorParams.shrink_to_fit();

        return null;
    }

    Value* Interpreter::visit_ClassInitializer(const Lexing::Token& name, const std::vector<Parsing::AST>& params) {
        if (auto classInit = currentScope->findSymbol(name.value)) {
            auto classVal = classInit->value;

            SymbolTable instanceScope{"instance-" + name.value + "-scope", {}, classVal->scope};

            auto newInstance = valueTable.addNewValue({
                .type=classInit,
                .val=classVal,
                .kind=InstanceVal,
                .ownScope=instanceScope
            });
            newInstance->important = true;

            std::vector<Value *> newParams;
            newParams.reserve(params.size());
            for (const auto& v : params) newParams.push_back(visit(v));
            constructorParams = newParams;

            auto tempScope = currentScope;
            currentScope = &newInstance->ownScope;

            auto thisSym = currentScope->addSymbol({
               classInit,
               "this",
               newInstance
            });

            auto currentClass = classVal;
            AST myInstanceBody = {
                .tp=InstanceBody,
                .lst_AST=std::any_cast<AST>(currentClass->val).lst_AST
            };

            auto mainScope = new SymbolTable{
                "inherited-scope-0",
                {{"this", *thisSym}},
                classVal->scope
            };
            std::vector<AST> inheritedBody = {std::move(myInstanceBody)};
            std::vector<SymbolTable*> inheritedScopes = {mainScope};

            int level = 1;
            while (currentClass->type->tp != nullptr) {
                auto upperValue = currentClass->type->tp->value;

                currentClass = upperValue;
                AST inherBody = {
                    .tp=InstanceBody,
                    .lst_AST=std::any_cast<AST>(currentClass->val).lst_AST
                };

                // This process would break here because setting the parent takes the address of a local object.
                // I need to clean this up. Raw pointers are gonna be a memory leak for a while.
                auto inherScope =new SymbolTable{
                    "inherited-scope-" + std::to_string(level++),
                    {{"this", *thisSym}},
                    inheritedScopes[0]
                };

                inheritedBody.push_back(inherBody);
                inheritedScopes.insert(inheritedScopes.begin(), inherScope);
            }

            newInstance->ownScope.setParent(inheritedScopes[0]);

            for (auto i = (long)inheritedBody.size()-1; i >= 0; i--) {
                auto prev = currentScope;

                currentScope = inheritedScopes[i];
                visit(inheritedBody[i]);

                currentScope = prev;
            }

            auto initID = Lexing::Token {Lexing::ID, "constructor"};
            AST initFuncCall = {
                ConstructorCall,
                initID
            };

            visit(initFuncCall);
//            std::any_cast<Value*>(newInstance->val);

            currentScope = tempScope;

            newInstance->important = false;

            return newInstance;
        } else {
            throw Exceptions::NameException(
                name.value + " is not a valid constructor.",
                current_line,
                current_col
            );
        }
    }

    Value* Interpreter::visit_InstanceBody(const std::vector<Parsing::AST>& statements){
        for (auto& st : statements) {
            if (st.tp != StaticStatement)
                visit(st);
        }

        return null;
    }

    Value* Interpreter::visit_MemberVar(const Parsing::AST& inst, const Lexing::Token& name) {
        auto instance = visit(inst);

        if (!instance || instance->kind != InstanceVal) {
            throw Exceptions::ValueException(
                "Invalid instance for Member Variable operator.",
                current_line,
                current_col
            );
        }

        auto foundSymbol = instance->ownScope.findSymbol(name.value);
        if (foundSymbol) {
            if (foundSymbol->value) return foundSymbol->value;
        } else {
            throw Exceptions::NameException(
                "No member variable named '" + name.value + "' in '" + inst.token.value + "'.",
                current_line,
                current_col
            );
        }

        return null;
    }

    Value* Interpreter::visit_StaticVar(const Parsing::AST& inst, const Lexing::Token& name) {
        auto instance = visit(inst);

        if (instance->kind == InstanceVal) {
            auto classVal = std::any_cast<Value*>(instance->val);

            auto foundSymbol = classVal->ownScope.findSymbol(name.value);
            if (foundSymbol) {
                if (foundSymbol->value) return foundSymbol->value;

                return null;
            } else {
                throw Exceptions::NameException(
                    "No static variable named '" + name.value + "' in '" + inst.token.value + "'.",
                    current_line,
                    current_col
                );
            }
        } else if (instance->kind == ClassVal) {
            auto foundSymbol = instance->ownScope.findSymbol(name.value);
            if (foundSymbol) {
                if (foundSymbol->value) return foundSymbol->value;

                return null;
            } else {
                throw Exceptions::NameException(
                    "No static variable named '" + name.value + "' in class '" + inst.token.value + "'.",
                    current_line,
                    current_col
                );
            }
        } else if (instance->kind == ModuleVal) {
            auto sm = instance->ownScope.findSymbol(name.value, false);
            if (sm) {
                if (sm->value)
                    return sm->value;
                return null;
            } else {
                throw Exceptions::NameException(
                    "No variable named '" + name.value + "' in module '" + inst.token.value + "'.",
                    current_line,
                    current_col
                );
            }
        } else if (instance->kind == EnumVal) {
            auto sm = instance->ownScope.findSymbol(name.value, false);
            if (sm) {
                if (sm->value)
                    return sm->value;
                return null;
            } else {
                throw Exceptions::NameException(
                    "'" + name.value + "' is not a variant in enum '" + inst.token.value + "'.",
                    current_line,
                    current_col
                );
            }
        } else {
            throw Exceptions::ValueException(
                "Cannot read static variable from this value.",
                current_line,
                current_col
            );
        }

        return null;
    }

    Symbol* Interpreter::getMemberVarSymbol(AST mem) {
        Symbol* varSym = nullptr;

        switch (mem.tp) {
            case Variable:
                varSym = currentScope->findSymbol(mem.token.value);
                break;
            case MemberVar:
            {
                auto leftHandSym = getMemberVarSymbol(mem.nodes["inst"]);

                if (leftHandSym && leftHandSym->value) {
                    auto theValue = leftHandSym->value;
                    if (theValue->kind != InstanceVal) {
                        throw Exceptions::ValueException(
                            "'" + leftHandSym->name + "' is not a valid instance.",
                            current_line,
                            current_col
                        );
                    }

                    varSym = theValue->ownScope.findSymbol(mem.token.value);
                }
                break;
            }
            case StaticVar:
            {
                auto leftHandSym = getMemberVarSymbol(mem.nodes["inst"]);

                if (leftHandSym && leftHandSym->value) {
                    auto theValue = leftHandSym->value;
                    if (theValue->kind == ModuleVal) {
                        varSym = theValue->ownScope.findSymbol(mem.token.value, false);
                    } else if (theValue->kind == ClassVal) {
                        varSym = theValue->ownScope.findSymbol(mem.token.value);
                    } else if (theValue->kind == InstanceVal) {
                        auto classVal = std::any_cast<Value*>(theValue->val);

                        varSym = classVal->ownScope.findSymbol(mem.token.value);
                    } else {
                        throw Exceptions::NameException(
                                "Invalid Static Variable Operator (::).",
                                current_line,
                                current_col
                        );
                    }
                } else {
                    throw Exceptions::NameException(
                            "Unknown value in Static Variable Operator (::).",
                            current_line,
                            current_col
                    );
                }

                break;
            }
            case Index:
            {
                auto visited_source = visit(mem.nodes["val"]);

                if (visited_source->kind == ListVal) {
                    auto visited_indx = visit(mem.nodes["expr"]);
                    if (visited_indx->type->name == "int") {
                        auto& as_list = visited_source->as_list_symbol();
                        auto as_int = visited_indx->as_int();
                        return &as_list[as_int];
                    } else {
                        throw Exceptions::TypeException(
                            "Lists can only be indexed with integer values.",
                            current_line,
                            current_col
                        );
                    }
                } else {
                    throw Exceptions::ValueException(
                            "Assignment to invalid indexing. You can only assign to list indices.",
                            current_line,
                            current_col
                    );
                }
            }
            default:
                break;
        }

        return varSym;
    }

    void Interpreter::interpret(std::string code) {
        parser.set_text(std::move(code));

        auto root = parser.program();

        call_stack.push_back({"global", 1, 1});
        visit(root);
        call_stack.pop_back();
    }

    Value* Interpreter::eval(std::string code) {

        call_stack.push_back({"global", 1, 1});
        parser.set_text(std::move(code));

        auto statements = parser.program_content();

    //    try{
        currentScope = &replScope;

        Value* result = null;
        for (const auto& node : statements) {
            result = visit(node);
        }

        currentScope = &globalTable;
    //    }
        call_stack.pop_back();

        return result;
    }

    bool Interpreter::is_copyable(const Value* v) { return v->type->kind == PrimitiveType && !(v->type == null->type); }

    Value * Interpreter::interpret_as_module(const std::string &path, const Lexing::Token& name) {
        bool has_extension = ends_with(path, ".odo");

        std::string full_path = path;
        if (!has_extension)
            full_path += ".odo";
        auto filename = io::get_file_name(full_path);
        if (name.tp != Lexing::NOTHING)
            filename = name.value;

        if (currentScope->findSymbol(filename)) {
            throw Exceptions::NameException(
                "Symbol called " + filename + " already exists in this scope",
                current_line,
                current_col
            );
        }

        std::string code;
        try {
            code = io::read_file(full_path);
        } catch (Exceptions::IOException&) {
            std::string msg = "Cannot import module '" + full_path + "'.";
            throw Exceptions::FileException(msg, current_line, current_col);
        }
        Parsing::Parser pr;
        pr.set_text(code);

        auto body = pr.program_content();
        AST file_module = {
            .tp=Module,
            .token=Lexing::Token(Lexing::STR, filename),
            .lst_AST=body,
            .line_number=0,
            .column_number=0
        };

        call_stack.push_back({"module '" + full_path + "'", 1, 1});

        auto result = visit(file_module);

        call_stack.pop_back();
        return result;
    }

    std::vector<std::pair<Symbol, bool>> Interpreter::getParamTypes(const std::vector<AST>& params) {
        std::vector<std::pair<Symbol, bool>> ts;

        for (auto par : params) {
            switch (par.tp) {
                case Declaration:
                    if (auto ft = currentScope->findSymbol(par.type.value)) {
                        ts.emplace_back(*ft, par.nodes["initial"].tp == NoOp);
                    } else {
                        // TODO: Handle Error
                        // Error! Unknown type par.type.value
                        throw Exceptions::TypeException(
                                "Unknown type " + par.type.value + ".",
                                par.line_number,
                                par.column_number
                        );
                    }
                    break;
                case ListDeclaration:
                    // FIXME: List types are registered as their basetype and not as listtype
                    if (auto ft = currentScope->findSymbol(par.type.value)) {
                        ts.emplace_back(*ft, par.nodes["initial"].tp == NoOp);
                    } else {
                        throw Exceptions::TypeException(
                                "Unknown type " + par.type.value + ".",
                                par.line_number,
                                par.column_number
                        );
                    }
                    break;
                default:
                    // Error! Expected variable declaration inside function parenthesis.
                    throw Exceptions::SyntaxException(
                            "Expected parameter declaration in function parenthesis",
                            par.line_number,
                            par.column_number
                    );
                    break;
            }
        }

        return ts;
    }
}