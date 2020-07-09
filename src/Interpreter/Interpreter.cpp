//
// Created by Luis Gonzalez on 3/20/20.
//

#include "Interpreter/Interpreter.h"
#include "Exceptions/exception.h"
#include "IO/io.h"
#include "utils.h"


#include "Parser/AST/DoubleNode.h"
#include "Parser/AST/IntNode.h"
#include "Parser/AST/BoolNode.h"
#include "Parser/AST/StrNode.h"
#include "Parser/AST/TernaryOpNode.h"
#include "Parser/AST/BinOpNode.h"
#include "Parser/AST/UnaryOpNode.h"
#include "Parser/AST/NoOpNode.h"
#include "Parser/AST/VarDeclarationNode.h"
#include "Parser/AST/ListDeclarationNode.h"
#include "Parser/AST/VariableNode.h"
#include "Parser/AST/AssignmentNode.h"
#include "Parser/AST/ListExpressionNode.h"
#include "Parser/AST/BlockNode.h"
#include "Parser/AST/FuncExpressionNode.h"
#include "Parser/AST/FuncDeclNode.h"
#include "Parser/AST/FuncCallNode.h"
#include "Parser/AST/FuncBodyNode.h"
#include "Parser/AST/ReturnNode.h"
#include "Parser/AST/IfNode.h"
#include "Parser/AST/ForNode.h"
#include "Parser/AST/ForEachNode.h"
#include "Parser/AST/FoRangeNode.h"
#include "Parser/AST/WhileNode.h"
#include "Parser/AST/LoopNode.h"
#include "Parser/AST/ModuleNode.h"
#include "Parser/AST/ImportNode.h"
#include "Parser/AST/EnumNode.h"
#include "Parser/AST/ClassNode.h"
#include "Parser/AST/ClassBodyNode.h"
#include "Parser/AST/InstanceBodyNode.h"
#include "Parser/AST/ClassInitializerNode.h"
#include "Parser/AST/ConstructorDeclNode.h"
#include "Parser/AST/ConstructorCallNode.h"
#include "Parser/AST/StaticStatementNode.h"
#include "Parser/AST/MemberVarNode.h"
#include "Parser/AST/StaticVarNode.h"
#include "Parser/AST/IndexNode.h"


#include <cmath>
#include <iostream>
#include <utility>
#include <chrono>
#include <thread>

#define noop (void)0
// #define DEBUG_FUNCTIONS

#define NOT_IMPLEMENTED(...) \
    throw Exceptions::OdoException("Using function not yet implemented: " + std::string(#__VA_ARGS__), current_line, current_col)

namespace Odo::Interpreting {
    using namespace Parsing;

    Interpreter::Interpreter(Parser p): parser(std::move(p)) {
        auto any_symbol = Symbol{.name="any", .isType=true, .kind=SymbolType::PrimitiveType};

        std::map<std::string, Symbol> buildInTypes = {
            {"any", any_symbol}
        };
        globalTable = SymbolTable("global", buildInTypes);

        auto any_sym = &globalTable.symbols["any"];

        globalTable.symbols["int"] = {.tp=any_sym, .name="int", .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols["double"] = {.tp=any_sym, .name="double", .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols["string"] = {.tp=any_sym, .name="string", .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols["bool"] = {.tp=any_sym, .name="bool", .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols["pointer"] = {.tp=any_sym, .name="pointer", .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols["NullType"] = {.tp=any_sym, .name="NullType", .isType=true, .kind=SymbolType::PrimitiveType};

        currentScope = &globalTable;

        replScope = SymbolTable("repl", {}, &globalTable);

        valueTable = ValueTable();

        null = std::make_shared<NormalValue>(globalTable.findSymbol("NullType"), "null");
        valueTable.addNewValue(null);

        auto nullSym = globalTable.addSymbol({
            .tp = &globalTable.symbols["NullType"],
            .name = "null",
            .value = null
        });
        null->addReference(*nullSym);

        replScope.symbols["_"] = {.tp=any_sym, .name="_", .value=null, .isType=false, .kind=SymbolType::VarSymbol};

        returning = nullptr;

#ifdef DEBUG_FUNCTIONS
        add_native_function("valueAt", [&](auto values) {
            int a = values[0]->as_int();

            return &valueTable.value_map()[a];
        });
#endif

        add_native_function("print", [&](auto values) {
            for (const auto& v : values) {
                if (v)
                    std::cout << v->to_string();
            }
            // Might make printing slower... I don't know of a better way of doing this.
            std::cout.flush();
            return null;
        });

        add_native_function("println", [&](auto values) {
            for (const auto& v : values) {
                if (v)
                    std::cout << v->to_string();
            }
            std::cout << std::endl;
            return null;
        });

        add_native_function("move_cursor", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                auto nv = Value::as<NormalValue>(vals[0]);
                if (nv) {
                    std::cout << "\033[" << nv->as_int() << ";" << nv->as_int() << "H";
                }
            }

            return null;
        });

        add_native_function("factorial", [&](std::vector<std::shared_ptr<Value>> v){
            if (!v.empty() && v[0]->type->name == "int") {
                int arg1 = Value::as<NormalValue>(v[0])->as_int();
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
        });

        add_native_function("length", [&](std::vector<std::shared_ptr<Value>> v){
            if (!v.empty()) {
                auto arg = v[0];
                if (arg->type->name == "string") {
                    size_t len = Value::as<NormalValue>(arg)->as_string().size();
                    return create_literal((int)len);
                } else if (arg->kind() == ValueType::ListVal) {
                    size_t len = Value::as<ListValue>(arg)->as_list_value().size();
                    return create_literal((int)len);
                }
            }
            throw Exceptions::FunctionCallException(
                "length function requires a single argument of type string or list.",
                current_line,
                current_col
            );
        });

        add_native_function("fromAsciiCode", [&](std::vector<std::shared_ptr<Value>> vals){
            if (!vals.empty()) {
                int val = Value::as<NormalValue>(vals[0])->as_int();

                return create_literal(std::string(1, val));
            }

            return null;
        });

        add_native_function("toAsciiCode", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                char val = Value::as<NormalValue>(vals[0])->as_string()[0];

                return create_literal(static_cast<int>(val));
            }

            return null;
        });

        add_native_function("pow", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (vals.size() >= 2) {
                bool result_as_int = true;
                double a = 0, b = 0;

                auto normal_val_first = Value::as<NormalValue>(vals[0]);
                auto normal_val_second = Value::as<NormalValue>(vals[1]);

                if (normal_val_first && vals[0]->type->name == "double") {
                    a = normal_val_first->as_double();
                    result_as_int = false;
                } else if (normal_val_first && vals[0]->type->name == "int") {
                    a = normal_val_first->as_int();
                }

                if (normal_val_second->type->name == "double") {
                    b = normal_val_second->as_double();
                    result_as_int = false;
                } else if (normal_val_second->type->name == "int") {
                    b = normal_val_second->as_int();
                }

                if (result_as_int) {
                    return create_literal((int) trunc(pow(a, b)));
                } else {
                    return create_literal(pow(a, b));
                }
            }
            return null;
        });

        add_native_function("sqrt", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                double a = 0;

                auto normal_value = Value::as<NormalValue>(vals[0]);

                if (normal_value && normal_value->type->name == "double") {
                    a = normal_value->as_double();
                } else if (normal_value && normal_value->type->name == "int") {
                    a = normal_value->as_int();
                }

                return create_literal(sqrt(a));
            }
            return null;
        });
        add_native_function("sin", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                double a = 0;

                auto normal_value = Value::as<NormalValue>(vals[0]);

                if (normal_value && normal_value->type->name == "double") {
                    a = normal_value->as_double();
                } else if (normal_value && normal_value->type->name == "int") {
                    a = normal_value->as_int();
                }

                return create_literal(sin(a));
            }
            return null;
        });
        add_native_function("cos", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                double a = 0;

                auto normal_value = Value::as<NormalValue>(vals[0]);

                if (normal_value && normal_value->type->name == "double") {
                    a = normal_value->as_double();
                } else if (normal_value && normal_value->type->name == "int") {
                    a = normal_value->as_int();
                }

                return create_literal(cos(a));
            }
            return null;
        });

        add_native_function("floor", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (vals.size() == 1) {
                auto v1 = Value::as<NormalValue>(vals[0]);

                double v = 0;
                if (v1 && v1->type->name == "double") {
                    v = v1->as_double();
                } else if (v1 && v1->type->name == "int") {
                    v = v1->as_int();
                } else {
                    throw Exceptions::ValueException("floor function can only be called with numeric values");
                }

                return create_literal(static_cast<int>(floor(v)));
            }

            return null;
        });

        add_native_function("trunc", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (vals.size() == 1) {
                auto v1 = Value::as<NormalValue>(vals[0]);
                double v = 0;
                if (v1 && v1->type->name == "double") {
                    v = v1->as_double();
                } else if (v1 && v1->type->name == "int") {
                    v = v1->as_int();
                } else {
                    throw Exceptions::ValueException("trunc function can only be called with numeric values");
                }

                return create_literal(static_cast<int>(trunc(v)));
            }

            return null;
        });

        add_native_function("round", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                auto v1 = Value::as<NormalValue>(vals[0]);
                double v = 0;
                if (v1 && v1->type->name == "double") {
                    v = v1->as_double();
                } else if (v1 && v1->type->name == "int") {
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

                auto v2 = Value::as<NormalValue>(vals[1]);
                if (!v2 || v2->type->name != "int") {
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

        add_native_function("read", [&](const std::vector<std::shared_ptr<Value>>& vals) {
            std::string result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }

            std::getline(std::cin, result);
            return create_literal(result);
        });

        add_native_function("read_int", [&](const std::vector<std::shared_ptr<Value>>& vals) {
            int result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(result);
        });

        add_native_function("read_double", [&](const std::vector<std::shared_ptr<Value>>& vals) {
            double result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(result);
        });

        add_native_function("rand", [&](std::vector<std::shared_ptr<Value>> vals) {
            double min = 0.0;
            double max = 1.0;
            if (vals.size() == 1) {
                auto normal_value_first = Value::as<NormalValue>(vals[0]);
                if (vals[0]->type->name == "double") {
                    max = normal_value_first->as_double();
                } else if (vals[0]->type->name == "int") {
                    max = normal_value_first->as_int();
                }
            } else if (vals.size() >= 2) {
                auto normal_value_first = Value::as<NormalValue>(vals[0]);
                if (vals[0]->type->name == "double") {
                    min =normal_value_first->as_double();
                } else if (vals[0]->type->name == "int") {
                    min = normal_value_first->as_int();
                }

                auto normal_value_second = Value::as<NormalValue>(vals[2]);
                if (vals[1]->type->name == "double") {
                    max = normal_value_second->as_double();
                } else if (vals[1]->type->name == "int") {
                    max = normal_value_second->as_int();
                }
            }

            return create_literal(rand_double(min, max));
        });

        add_native_function("randInt", [&](std::vector<std::shared_ptr<Value>> vals) {
            int min = 0;
            int max = INT32_MAX;
            if (vals.size() == 1) {
                max = Value::as<NormalValue>(vals[0])->as_int();
            } else if (vals.size() >= 2) {
                min = Value::as<NormalValue>(vals[0])->as_int();
                max = Value::as<NormalValue>(vals[1])->as_int();
            }

            return create_literal(rand_int(min, max));
        });

        add_native_function("pop", [&](std::vector<std::shared_ptr<Value>> vals) {
            if (vals.size() == 1) {
                auto& lst = vals[0];
                if (lst->kind() == ValueType::ListVal) {
                    std::vector<Symbol>& syms = Value::as<ListValue>(lst)->elements;

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

        add_native_function("sleep", [&](std::vector<std::shared_ptr<Value>> vals){
            if (!vals.empty()) {
                auto delay_time = Value::as<NormalValue>(vals[0])->as_int();

                std::this_thread::sleep_for(std::chrono::milliseconds(delay_time));
            }
            return null;
        });
    }

    Symbol* Interpreter::any_type() {
        return &globalTable.symbols["any"];
    }

    std::pair<std::shared_ptr<Value>, std::shared_ptr<Value>>
    Interpreter::coerce_type(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs) {
        auto result = std::pair{lhs, rhs};
        if (lhs->type->kind != SymbolType::PrimitiveType || rhs->type->kind != SymbolType::PrimitiveType)
            return result;

        // Check if both values are numerical
        if (lhs->type->name != rhs->type->name && lhs->is_numeric() && rhs->is_numeric()) {
            auto left_numeric = Value::as<NormalValue>(lhs);
            auto right_numeric = Value::as<NormalValue>(rhs);

            if (lhs->type->name == "int") {
                auto new_left = create_literal((double)left_numeric->as_int());

                result.first = std::move(new_left);
            }

            if (rhs->type->name == "int") {
                auto new_left = create_literal((double)right_numeric->as_int());

                result.second = std::move(new_left);
            }
        }

        return result;
    }

    std::shared_ptr<Value> Interpreter::visit(const std::shared_ptr<Node>& node) {
        current_line = node->line_number;
        current_col = node->column_number;
        switch (node->kind()) {
            // Normal Types
            case NodeType::Double:
                return visit_Double(Node::as<DoubleNode>(node));
            case NodeType::Int:
                return visit_Int(Node::as<IntNode>(node));
            case NodeType::Bool:
                return visit_Bool(Node::as<BoolNode>(node));
            case NodeType::Str:
                return visit_Str(Node::as<StrNode>(node));

            // Operations
            case NodeType::BinOp:
                return visit_BinOp(Node::as<BinOpNode>(node));
            case NodeType::UnaryOp:
                return visit_UnaryOp(Node::as<UnaryOpNode>(node));
            case NodeType::NoOp:
                return null;

            case NodeType::Index:
                return visit_Index(Node::as<IndexNode>(node));

            // Control Flow
            case NodeType::TernaryOp:
                return visit_TernaryOp(Node::as<TernaryOpNode>(node));
            case NodeType::If:
                return visit_If(Node::as<IfNode>(node));
            case NodeType::For:
                return visit_For(Node::as<ForNode>(node));
            case NodeType::ForEach:
                return visit_ForEach(Node::as<ForEachNode>(node));
            case NodeType::FoRange:
                return visit_FoRange(Node::as<FoRangeNode>(node));
            case NodeType::While:
                return visit_While(Node::as<WhileNode>(node));
            case NodeType::Loop:
                return visit_Loop(Node::as<LoopNode>(node));
            case NodeType::Break:
                breaking = true;
                return null;
            case NodeType::Continue:
                continuing = true;
                return null;
            case NodeType::Block:
                return visit_Block(Node::as<BlockNode>(node));

            // Variable Handling
            case NodeType::VarDeclaration:
                return visit_VarDeclaration(Node::as<VarDeclarationNode>(node));
            case NodeType::ListDeclaration:
                return visit_ListDeclaration(Node::as<ListDeclarationNode>(node));
            case NodeType::Variable:
                return visit_Variable(Node::as<VariableNode>(node));
            case NodeType::Assignment:
                return visit_Assignment(Node::as<AssignmentNode>(node));

            case NodeType::ListExpression:
                return visit_ListExpression(Node::as<ListExpressionNode>(node));

            // Functions
            case NodeType::FuncExpression:
                return visit_FuncExpression(Node::as<FuncExpressionNode>(node));
            case NodeType::FuncDecl:
                return visit_FuncDecl(Node::as<FuncDeclNode>(node));
            case NodeType::FuncCall:
                return visit_FuncCall(Node::as<FuncCallNode>(node));
            case NodeType::FuncBody:
                return visit_FuncBody(Node::as<FuncBodyNode>(node));
            case NodeType::Return:
                return visit_Return(Node::as<ReturnNode>(node));

            case NodeType::Enum:
                return visit_Enum(Node::as<EnumNode>(node));

            // Classes
            case NodeType::Class:
                return visit_Class(Node::as<ClassNode>(node));
            case NodeType::ClassBody:
                return visit_ClassBody(Node::as<ClassBodyNode>(node));
//          Broken or incomplete.
            case NodeType::ConstructorDecl:
                return visit_ConstructorDecl(Node::as<ConstructorDeclNode>(node));
            case NodeType::ConstructorCall:
                return visit_ConstructorCall(Node::as<ConstructorCallNode>(node));
            case NodeType::InstanceBody:
                return visit_InstanceBody(Node::as<InstanceBodyNode>(node));
            case NodeType::ClassInitializer:
                return visit_ClassInitializer(Node::as<ClassInitializerNode>(node));
            case NodeType::StaticStatement:
                throw Exceptions::OdoException(
                    "Static statements can only appear inside a class body.",
                    current_line,
                    current_col
                );
            case NodeType::MemberVar:
                return visit_MemberVar(Node::as<MemberVarNode>(node));
            case NodeType::StaticVar:
                return visit_StaticVar(Node::as<StaticVarNode>(node));

            case NodeType::Module:
                return visit_Module(Node::as<ModuleNode>(node));
            case NodeType::Import:
                return visit_Import(Node::as<ImportNode>(node));

            case NodeType::Debug:
                noop;
            case NodeType::Null:
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

    std::shared_ptr<Value> Interpreter::create_literal_from_string(std::string val, const std::string& kind) {
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

        auto normal_value = std::make_shared<NormalValue>(globalTable.findSymbol(kind), newValue);

        valueTable.addNewValue(normal_value);

        return normal_value;
    }

    std::shared_ptr<Value> Interpreter::create_literal_from_any(const std::any& val, const std::string &kind) {
        auto the_value = std::make_shared<NormalValue>(globalTable.findSymbol(kind), val);
        valueTable.addNewValue(the_value);

        return std::move(the_value);
    }

    std::shared_ptr<Value> Interpreter::create_literal(std::string val) {
        return create_literal_from_any(val, "string");
    }

    std::shared_ptr<Value> Interpreter::create_literal(int val) {
        return create_literal_from_any(val, "int");
    }

    std::shared_ptr<Value> Interpreter::create_literal(double val) {
        return create_literal_from_any(val, "double");
    }

    std::shared_ptr<Value> Interpreter::create_literal(bool val) {
        return create_literal_from_any(val, "bool");
    }

    std::shared_ptr<Value> Interpreter::visit_Double(const std::shared_ptr<DoubleNode>& node) {
        return create_literal_from_string(node->token.value, "double");
    }

    std::shared_ptr<Value> Interpreter::visit_Int(const std::shared_ptr<IntNode>& node) {
        return create_literal_from_string(node->token.value, "int");
    }

    std::shared_ptr<Value> Interpreter::visit_Bool(const std::shared_ptr<BoolNode>& node) {
        return create_literal_from_string(node->token.value, "bool");
    }

    std::shared_ptr<Value> Interpreter::visit_Str(const std::shared_ptr<StrNode>& node) {
        return create_literal_from_string(node->token.value, "string");
    }

    std::shared_ptr<Value> Interpreter::visit_Block(const std::shared_ptr<BlockNode>& node) {
        auto blockScope = SymbolTable("block_scope", {}, currentScope);
        currentScope = &blockScope;

        auto result = null;
        for (const auto& st : node->statements) {
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

    std::shared_ptr<Value> Interpreter::visit_TernaryOp(const std::shared_ptr<TernaryOpNode>& node) {
        auto val_cond = visit(node->cond);
        if (val_cond->type->name != "bool") {
            throw Exceptions::TypeException(
                    "Condition of ternary expression must be boolean.",
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        bool real_condition = Value::as<NormalValue>(val_cond)->as_bool();

        if (real_condition) {
            return visit(node->trueb);
        } else {
            return visit(node->falseb);
        }
        return nullptr;
    }

    std::shared_ptr<Value> Interpreter::visit_If(const std::shared_ptr<IfNode>& node) {
        auto val_cond = visit(node->cond);
        if (val_cond->type->name != "bool") {
            throw Exceptions::TypeException(
                    "Condition of if statement must be boolean.",
                    node->cond->line_number,
                    node->cond->column_number
            );
        }
        bool real_condition = Value::as<NormalValue>(val_cond)->as_bool();

        if (real_condition) {
            return visit(node->trueb);
        } else if (node->falseb) {
            return visit(node->falseb);
        }
        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_For(const std::shared_ptr<ForNode>& node) {
        auto forScope = SymbolTable("for:loop", {}, currentScope);
        currentScope = &forScope;

        visit(node->ini);

        auto val_cond = visit(node->cond);
        if (val_cond->type->name != "bool") {
            throw Exceptions::TypeException(
                    "Condition expression of for statement must be boolean.",
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        auto actual_cond = Value::as<NormalValue>(val_cond)->as_bool();
        while(actual_cond){
            if (continuing) {
                continuing = false;
                continue;
            } else {
                visit(node->body);
            }

            if (breaking) {
                breaking = false;
                break;
            }

            if (returning) {
                break;
            }

            visit(node->incr);

            val_cond = visit(node->cond);
            actual_cond = Value::as<NormalValue>(val_cond)->as_bool();
        }

        continuing = false;

        currentScope = forScope.getParent();
        valueTable.cleanUp(forScope);

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_ForEach(const std::shared_ptr<ForEachNode>& node) {
        auto forScope = SymbolTable("foreach:loop", {}, currentScope);
        currentScope = &forScope;

        auto lst_value = visit(node->lst);
        if (lst_value->kind() == ValueType::ListVal) {
            std::shared_ptr<Node> iterator_decl;
            auto empty_initial = std::make_shared<NoOpNode>();
            auto element_tp = Lexing::Token(Lexing::TokenType::ID, lst_value->type->name);
            if (lst_value->type->tp && lst_value->type->tp->kind == SymbolType::ListType) {
                iterator_decl = std::make_shared<ListDeclarationNode>(
                    std::move(element_tp),
                    node->var,
                    std::move(empty_initial)
                );
            } else {
                iterator_decl = std::make_shared<VarDeclarationNode>(std::move(element_tp), node->var, std::move(empty_initial));
            }

            // TODO: Add debugging info, like line and column number.
            visit(iterator_decl);

            auto declared_iter = currentScope->findSymbol(node->var.value);

            auto& the_symbols = Value::as<ListValue>(lst_value)->elements;

            bool go_backwards = node->rev.tp != Lexing::NOTHING;
            lst_value->important = true;

            for(size_t i = 0; i < the_symbols.size(); i++){
                auto actual_index = i;
                if (go_backwards) {
                    actual_index = the_symbols.size()-1-i;
                }

                auto& s = the_symbols[actual_index];
                declared_iter->value = s.value;

                visit(node->body);
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
            lst_value->important = false;
        } else if (lst_value->type->name == "string") {
            auto iterator_decl = std::make_shared<VarDeclarationNode>(
                Lexing::Token(Lexing::TokenType::ID, "string"),
                node->var,
                std::make_shared<NoOpNode>()
            );
            visit(iterator_decl);

            auto declared_iter = currentScope->findSymbol(node->var.value);
            declared_iter->value = create_literal(std::string(1, '\0'));
            declared_iter->value->addReference(*declared_iter);

            auto iter_as_normal = Value::as<NormalValue>(declared_iter->value);

            auto st = Value::as<NormalValue>(lst_value)->as_string();

            bool go_backwards = node->rev.tp != Lexing::NOTHING;

            for(size_t i = 0; i < st.size(); i++){
                size_t actual_index = i;
                if (go_backwards) actual_index = st.size() - 1 - i;

                char s = st[actual_index];
                iter_as_normal->val = std::string(1, s);

                visit(node->body);
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
            declared_iter->value->removeReference(*declared_iter);
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

    std::shared_ptr<Value> Interpreter::visit_FoRange(const std::shared_ptr<FoRangeNode>& node){
        auto forScope = SymbolTable("forange:loop", {}, currentScope);
        currentScope = &forScope;

        int max_in_range = 0;
        auto first_visited = visit(node->first);
        if (!first_visited->is_numeric()) {
            throw Exceptions::ValueException(
                "Values defining the range of forange statement have to be numerical",
                current_line,
                current_col
            );
        }

        if (first_visited->type->name == "int")
            max_in_range = Value::as<NormalValue>(first_visited)->as_int();
        else if (first_visited->type->name == "double")
            max_in_range = floor(Value::as<NormalValue>(first_visited)->as_double());

        int min_in_range = 0;

        if (node->second && node->second->kind() != NodeType::NoOp) {
            min_in_range = max_in_range;

            auto second_visited = visit(node->second);
            if (!second_visited->is_numeric()) {
                throw Exceptions::ValueException(
                    "Values defining the range of forange statement have to be numerical",
                    current_line,
                    current_col
                );
            }

            if (second_visited->type->name == "int")
                max_in_range = Value::as<NormalValue>(second_visited)->as_int();
            else if (second_visited->type->name == "double")
                max_in_range = floor(Value::as<NormalValue>(second_visited)->as_double());
        }

        bool go_backwards = node->rev.tp != Lexing::NOTHING;

        auto iterator_decl = std::make_shared<VarDeclarationNode>(
            Lexing::Token(Lexing::TokenType::ID, "int"),
            node->var,
            std::make_shared<NoOpNode>()
        );
        visit(iterator_decl);

        auto declared_iter = currentScope->findSymbol(node->var.value);
        declared_iter->value = create_literal(0);
        declared_iter->value->addReference(*declared_iter);

        auto iter_as_normal = Value::as<NormalValue>(declared_iter->value);

        for(int i = min_in_range; i < max_in_range; i++){
            auto actual_value = i;
            if (go_backwards) actual_value = min_in_range + max_in_range-1-i;

            iter_as_normal->val = actual_value;

            visit(node->body);
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
        declared_iter->value->removeReference(*declared_iter);
        currentScope = forScope.getParent();
        valueTable.cleanUp(forScope);

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_While(const std::shared_ptr<WhileNode>& node) {
        auto whileScope = SymbolTable("while:loop", {}, currentScope);
        currentScope = &whileScope;

        auto val_cond = visit(node->cond);
        if (val_cond->type->name != "bool") {
            throw Exceptions::TypeException(
                    "Condition expression of for statement must be boolean.",
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        auto actual_cond = Value::as<NormalValue>(val_cond)->as_bool();
        while(actual_cond){
            visit(node->body);
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

            actual_cond = Value::as<NormalValue>(visit(node->cond))->as_bool();
        }

        currentScope = whileScope.getParent();
        valueTable.cleanUp(whileScope);

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_Loop(const std::shared_ptr<LoopNode>& node) {
        while(true){
            visit(node->body);
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

    std::shared_ptr<Value> Interpreter::visit_VarDeclaration(const std::shared_ptr<VarDeclarationNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    "Variable called '" + node->name.value + "' already exists",
                    current_line,
                    current_col
            );
        } else {
            auto type_ = currentScope->findSymbol(node->var_type.value);

            if (type_ == nullptr) {
                throw Exceptions::NameException(
                        "Unknown type '" + node->var_type.value + "'.",
                        current_line,
                        current_col
                );
            }

            Symbol newVar;
            std::shared_ptr<Value> valueReturn;

            if (node->initial && node->initial->kind() != NodeType::NoOp) {
                auto newValue = visit(node->initial);

                if (newValue->is_copyable()) {
                    newValue = newValue->copy(valueTable);
                }

                if (type_->name == "any") {
                    type_ = newValue->type;
                } else {
                    if (type_->name == "int" && newValue->type->name == "double") {
                        newValue = create_literal((int) Value::as<NormalValue>(newValue)->as_double());
                    } else if (type_->name == "double" && newValue->type->name == "int") {
                        newValue = create_literal((double) Value::as<NormalValue>(newValue)->as_int());
                    }
                }

                newVar = {
                    type_,
                    node->name.value,
                    newValue
                };

                valueReturn = std::move(newValue);
            } else {
                newVar = {
                    type_,
                    node->name.value
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

    std::shared_ptr<Value> Interpreter::visit_ListDeclaration(const std::shared_ptr<ListDeclarationNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    "Variable called '" + node->name.value + "' already exists",
                    current_line,
                    current_col
            );
        }

        auto base_type = currentScope->findSymbol(node->var_type.value);
        if (!(base_type && base_type->isType)) {
            throw Exceptions::TypeException(
                    "Invalid type '" + node->var_type.value + "'.",
                    current_line,
                    current_col
            );
        }

        Symbol* newVar;
        std::shared_ptr<Value> valueReturn = null;

        Symbol* list_type;

        auto found_in_table = globalTable.findSymbol(base_type->name + "[]");
        if (found_in_table) {
            list_type = found_in_table;
        } else {
            list_type = globalTable.addSymbol({
                .tp=base_type,
                .name=base_type->name + "[]",
                .isType=true,
                .kind=SymbolType::ListType
            });
        }

        if (node->initial && node->initial->kind() != NodeType::NoOp) {
            auto newValue = visit(node->initial);

            // Well... Given the nature of my language right now
            // I shouldn't care about this.
            // TODO: But this is surely a warning.
            if (newValue->kind() != ValueType::ListVal) noop;

            newVar = currentScope->addSymbol({
                list_type,
                node->name.value,
                newValue
            });

            newValue->addReference(*newVar);
            valueReturn = newValue;

            return valueReturn;
        }

        currentScope->addSymbol({
            list_type,
            node->name.value
        });

        return valueReturn;
    }

    std::shared_ptr<Value> Interpreter::visit_Assignment(const std::shared_ptr<AssignmentNode>& node) {
        auto varSym = getMemberVarSymbol(node->expr);
        auto newValue = visit(node->val);

        if (varSym) {
            if (varSym->value) {
                auto theValue = varSym->value;

                theValue->removeReference(*varSym);
                if (newValue->is_copyable()) {
                    newValue = newValue->copy(valueTable);
                }
            } else {
                if (newValue->is_copyable()) {
                    newValue = newValue->copy(valueTable);
                }

                if (varSym->tp->name == "any") {
                    varSym->tp = newValue->type;
                } else {
                    if (varSym->tp->name == "int" && newValue->type->name == "double") {
                        newValue = create_literal((int) Value::as<NormalValue>(newValue)->as_double());
                    } else if (varSym->tp->name == "double" && newValue->type->name == "int") {
                        newValue = create_literal((double) Value::as<NormalValue>(newValue)->as_int());
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

    std::shared_ptr<Value> Interpreter::visit_Variable(const std::shared_ptr<VariableNode>& node) {
        auto found = currentScope->findSymbol(node->token.value);

        if (found != nullptr) {
            if (found->value) {
                return (found->value == nullptr) ? null : found->value;
            } else {
                return null;
            }
        } else {
            throw Exceptions::NameException(
                    "Variable named '" + node->token.value + "' not defined.",
                    current_line,
                    current_col
            );
        }
    }

    std::shared_ptr<Value> Interpreter::visit_Index(const std::shared_ptr<IndexNode>& node) {
        auto visited_val = visit(node->val);

        if (visited_val->type->name == "string") {
            auto str = Value::as<NormalValue>(visited_val)->as_string();

            auto visited_indx = visit(node->expr);
            if (visited_indx->type->name == "int") {
                auto int_indx = Value::as<NormalValue>(visited_indx)->as_int();

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
        } else if (visited_val->kind() == ValueType::ListVal){
            auto list_value = Value::as<ListValue>(visited_val)->as_list_value();
            auto visited_indx = visit(node->expr);
            if (visited_indx->type->name == "int") {
                auto int_indx = Value::as<NormalValue>(visited_indx)->as_int();

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

    std::shared_ptr<Value> Interpreter::visit_ListExpression(const std::shared_ptr<ListExpressionNode>& node) {
        Symbol* list_t = nullptr;
        std::vector<Symbol> list_syms;

        for (const auto& el : node->elements) {
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

            std::shared_ptr<Value> actual_value;

            if (visited_element->is_copyable()) {
                actual_value = visited_element->copy(valueTable);
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

        auto new_list_value = std::make_shared<ListValue>(list_type, std::move(list_syms));
        valueTable.addNewValue(new_list_value);

        return new_list_value;
    }

    std::shared_ptr<Value> Interpreter::visit_BinOp(const std::shared_ptr<BinOpNode>& node) {
        auto leftVisited = visit(node->left);
        leftVisited->important = true;
        auto rightVisited = visit(node->right);
        leftVisited->important = false;

        auto coerced = coerce_type(leftVisited, rightVisited);
        leftVisited = coerced.first;
        rightVisited = coerced.second;

        switch (node->token.tp) {
            case Lexing::PLUS: {
                auto left_as_normal = Value::as<NormalValue>(leftVisited);
                auto right_as_normal = Value::as<NormalValue>(rightVisited);
                if (leftVisited->kind() == ValueType::ListVal) {
                    auto left_as_list = Value::as<ListValue>(leftVisited);
                    std::shared_ptr<Value> new_list;

                    if (rightVisited->kind() == ValueType::ListVal) {
                        auto right_as_list = Value::as<ListValue>(rightVisited);
                        std::vector<Symbol> new_elements;

                        for (const auto &el : left_as_list->elements) {
                            auto val = el.value;

                            if (val->is_copyable()) {
                                val = val->copy(valueTable);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }
                        for (const auto &el : right_as_list->elements) {
                            auto val = el.value;

                            if (val->is_copyable()) {
                                val = val->copy(valueTable);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }

                        new_list = std::make_shared<ListValue>(leftVisited->type, std::move(new_elements));
                        valueTable.addNewValue(new_list);

                        return new_list;
                    } else {
                        std::vector<Symbol> new_elements;

                        for (const auto &el : left_as_list->elements) {
                            auto val = el.value;

                            if (val->is_copyable()) {
                                val = val->copy(valueTable);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }

                        auto val = rightVisited;
                        if (val->is_copyable()) {
                            val = val->copy(valueTable);
                        }

                        Symbol new_symbol = {val->type, "list_element", val};
                        val->addReference(new_symbol);
                        new_elements.push_back(new_symbol);

                        new_list = std::make_shared<ListValue>(
                                globalTable.addListType(val->type),
                                std::move(new_elements)
                        );
                        valueTable.addNewValue(new_list);
                        return new_list;
                    }
                } else if (leftVisited->type->name == "string") {
                    return create_literal(left_as_normal->as_string() + rightVisited->to_string());
                } else if (rightVisited->type->name == "string") {
                    return create_literal(leftVisited->to_string() + right_as_normal->as_string());
                } else if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() + right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() + right_as_normal->as_double();
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
            }
            case Lexing::MINUS:
                if (leftVisited->type == rightVisited->type) {
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() - right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() - right_as_normal->as_double();
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
            case Lexing::MUL: {
                auto left_as_normal = Value::as<NormalValue>(leftVisited);
                auto right_as_normal = Value::as<NormalValue>(rightVisited);
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() * right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() * right_as_normal->as_double();
                        return create_literal(result);
                    }
                } else if (leftVisited->kind() == ValueType::ListVal && rightVisited->type->name == "int") {
                    int right_as_int = right_as_normal->as_int();
                    std::vector<Symbol> new_elements;

                    for (int i = 0; i < right_as_int; i++) {
                        for (const auto &el : Value::as<ListValue>(leftVisited)->elements) {
                            auto val = el.value;

                            if (val->is_copyable()) {
                                val = val->copy(valueTable);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }
                    }

                    auto new_list = std::make_shared<ListValue>(leftVisited->type, std::move(new_elements));
                    valueTable.addNewValue(new_list);

                    return new_list;
                } else if (leftVisited->type->name == "string" && rightVisited->type->name == "int") {
                    std::string left_as_string = left_as_normal->as_string();
                    int right_as_int = right_as_normal->as_int();
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
            }
            case Lexing::DIV: {
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == "double") {
                    auto result = Value::as<NormalValue>(leftVisited)->as_double() / Value::as<NormalValue>(rightVisited)->as_double();
                    return create_literal(result);
                } else {
                    throw Exceptions::TypeException(
                            "Division operation can only be used with values of type double.",
                            current_line,
                            current_col
                    );
                }
                break;
            }
            case Lexing::MOD:
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == "int") {
                    auto result = Value::as<NormalValue>(leftVisited)->as_int() % Value::as<NormalValue>(rightVisited)->as_int();
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
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == "int") {
                        auto result = powl(left_as_normal->as_int(), right_as_normal->as_int());
                        return create_literal((double)result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = powl(left_as_normal->as_double(), right_as_normal->as_double());
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
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() == right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() == right_as_normal->as_double();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "bool") {
                        auto result = left_as_normal->as_bool() == right_as_normal->as_bool();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "string") {
                        auto result = left_as_normal->as_string() == right_as_normal->as_string();
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
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() != right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() != right_as_normal->as_double();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "bool") {
                        auto result = left_as_normal->as_bool() != right_as_normal->as_bool();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "string") {
                        auto result = left_as_normal->as_string() != right_as_normal->as_string();
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
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() < right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() < right_as_normal->as_double();
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
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() > right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() > right_as_normal->as_double();
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
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() <= right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() <= right_as_normal->as_double();
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
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == "int") {
                        auto result = left_as_normal->as_int() >= right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == "double") {
                        auto result = left_as_normal->as_double() >= right_as_normal->as_double();
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
                    auto result = Value::as<NormalValue>(leftVisited)->as_bool() && Value::as<NormalValue>(rightVisited)->as_bool();
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
                    auto result = Value::as<NormalValue>(leftVisited)->as_bool() || Value::as<NormalValue>(rightVisited)->as_bool();
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

    std::shared_ptr<Value> Interpreter::visit_UnaryOp(const std::shared_ptr<UnaryOpNode>& node) {
        auto result = visit(node->ast);

        auto result_as_normal = Value::as<NormalValue>(result);

        switch (node->token.tp) {
            case Lexing::PLUS:
                break;
            case Lexing::MINUS:
                if (result->type->name == "int") {
                    auto actual_value = result_as_normal->as_int();

                    result_as_normal->val = actual_value * -1;
                    return result;
                }

                if (result->type->name == "double") {
                    auto actual_value = result_as_normal->as_double();

                    result_as_normal->val = actual_value * -1;
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

    std::shared_ptr<Value> Interpreter::visit_Module(const std::shared_ptr<ModuleNode>& node) {
        SymbolTable module_scope = {
            "module-scope",
            {},
            currentScope
        };

        auto moduleValue = std::make_shared<ModuleValue>(null->type, std::move(module_scope));
        valueTable.addNewValue(moduleValue);
        moduleValue->important = true;

        auto temp = currentScope;
        currentScope = &moduleValue->ownScope;
        for (const auto& st : node->statements) {
            visit(st);
        }

        currentScope = temp;
        moduleValue->important = false;

        auto module_sym = currentScope->addSymbol({nullptr, node->name.value, moduleValue});
        moduleValue->addReference(*module_sym);

        return moduleValue;
    }

    std::shared_ptr<Value> Interpreter::visit_Import(const std::shared_ptr<ImportNode>& node) {
        interpret_as_module(node->path.value, node->name);
        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_FuncExpression(const std::shared_ptr<FuncExpressionNode>& node){
        auto returnType =
                node->retType.tp == Lexing::NOTHING
                ? nullptr
                : currentScope->findSymbol(node->retType.value);

        auto paramTypes = getParamTypes(node->params);

        auto typeName = Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(returnType, paramTypes);
        }

        auto funcValue = std::make_shared<FunctionValue>(typeOfFunc, node->params, node->body, currentScope);
        valueTable.addNewValue(funcValue);

        return funcValue;
    }

    std::shared_ptr<Value> Interpreter::visit_FuncDecl(const std::shared_ptr<FuncDeclNode>& node){

        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    "Variable called '" + node->name.value + "' already exists",
                    current_line,
                    current_col
            );
        }

        auto returnType =
                node->retType.tp == Lexing::NOTHING
                ? nullptr
                : currentScope->findSymbol(node->retType.value);

        auto paramTypes = getParamTypes(node->params);

        auto typeName = Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(returnType, paramTypes);
        }

        auto funcValue = std::make_shared<FunctionValue>(typeOfFunc, node->params, node->body, currentScope);
        valueTable.addNewValue(funcValue);

        auto funcSymbol = currentScope->addSymbol({
            .tp=typeOfFunc,
            .name=node->name.value,
            .value=funcValue,
            .kind=SymbolType::FunctionSymbol
        });

        funcValue->addReference(*funcSymbol);

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_FuncCall(const std::shared_ptr<FuncCallNode>& node) {
        if (call_stack.size() >= MAX_CALL_DEPTH) {
            throw Exceptions::RecursionException("Callback depth exceeded.", current_line, current_col);
        }

        if (node->fname.tp != Lexing::NOTHING){
            auto found_in_natives = native_functions.find(node->fname.value);

            if (found_in_natives != native_functions.end()) {
                auto num_args = node->args.size();
                std::vector<std::shared_ptr<Value>> arguments_visited;
                arguments_visited.reserve(num_args);

                std::vector<bool> was_important{};
                was_important.reserve(num_args);
                for(int i = 0; i < num_args; i++){
                    auto& arg = node->args[i];
                    auto v = visit(arg);
                    was_important[i] = v->important;
                    v->important = true;
                    arguments_visited.push_back(std::move(v));
                }

                auto result = found_in_natives->second(arguments_visited);
                for (int i = 0; i < arguments_visited.size(); i++) {
                    if (!was_important[i])
                        arguments_visited[i]->important = false;
                }

                return result;
            }
        }

        if (auto fVal = visit(node->expr);
            fVal->kind() == ValueType::FunctionVal) {
            auto as_function_value = Value::as<FunctionValue>(fVal);

            auto funcScope = SymbolTable("func-scope", {}, as_function_value->parentScope);
            auto calleeScope = currentScope;

            std::vector<std::shared_ptr<Node>> newDecls;
            std::vector< std::pair<Lexing::Token, std::shared_ptr<Value>> > initValues;

            for (int i = 0; i < as_function_value->params.size(); i++) {
                auto par = as_function_value->params[i];
                if (node->args.size() > i) {
                    switch (par->kind()) {
                        case NodeType::VarDeclaration:
                        {
                            auto newValue = visit(node->args[i]);
                            if (newValue->is_copyable()) {
                                newValue = newValue->copy(valueTable);
                            }
                            initValues.emplace_back(Node::as<VarDeclarationNode>(par)->name, newValue);
                            break;
                        }
                        case NodeType::ListDeclaration:
                        {
                            auto newValue = visit(node->args[i]);
                            initValues.emplace_back(Node::as<ListDeclarationNode>(par)->name, newValue);
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
                auto ref = *fVal->references.begin();
                function_name = ref->name;
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

            auto body_as_ast = as_function_value->body;

            auto result = visit(body_as_ast);
            currentScope = calleeScope;
            valueTable.cleanUp(funcScope);

            result->important = false;
            call_stack.pop_back();
            return result;
        }

        throw Exceptions::ValueException("Value is not a function.", current_line, current_col);
    }

    std::shared_ptr<Value> Interpreter::visit_FuncBody(const std::shared_ptr<FuncBodyNode>& node) {
        auto temp = currentScope;
        auto bodyScope = SymbolTable("func-body-scope", {}, currentScope);

        currentScope = &bodyScope;

        for (const auto& st : node->statements) {
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

    std::shared_ptr<Value> Interpreter::visit_Return(const std::shared_ptr<ReturnNode>& node) {
        returning = visit(node->val);
        returning->important = true;
        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_Enum(const std::shared_ptr<EnumNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    "Variable called '" + node->name.value + "' already exists",
                    current_line,
                    current_col
            );
        }

        Symbol newEnumSym = {
            .name=node->name.value,
            .isType=true,
            .kind=SymbolType::EnumType
        };

        Symbol* enumInTable = currentScope->addSymbol(newEnumSym);

        SymbolTable enum_variant_scope;

        for (const auto& variant : node->variants) {
            auto variant_name = Node::as<VariableNode>(variant)->token.value;

            auto variant_value = std::make_shared<EnumVarValue>(enumInTable, variant_name);
            valueTable.addNewValue(variant_value);

            auto in_scope = enum_variant_scope.addSymbol({
                enumInTable,
                variant_name,
                variant_value
            });

            variant_value->addReference(*in_scope);
        }

        auto newValue = std::make_shared<EnumValue>(null->type, std::move(enum_variant_scope));
        valueTable.addNewValue(newValue);
        newValue->addReference(*enumInTable);
        enumInTable->value = newValue;

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_Class(const std::shared_ptr<ClassNode>& node) {
        Symbol* typeSym = nullptr;

        if (node->ty.tp != Lexing::NOTHING) {
            auto sym = currentScope->findSymbol(node->ty.value);
            if (!sym || !sym->isType) {
                throw Exceptions::TypeException(
                        "Class must inherit from a type. " + node->name.value + " is invalid.",
                        current_line,
                        current_col
                );
            }

            typeSym = sym;
        }

        Symbol newClassSym = {
            .tp=typeSym,
            .name=node->name.value,
            .isType = true,
            .kind = SymbolType::ClassType,
        };
        auto inTable = currentScope->addSymbol(newClassSym);

        SymbolTable classScope{"class-" + node->name.value + "-scope", {}, currentScope};
        auto newClassMolde = std::make_shared<ClassValue>(inTable, classScope, currentScope, node->body);
        valueTable.addNewValue(newClassMolde);

        auto prevScope = currentScope;
        currentScope = &newClassMolde->ownScope;

        visit(node->body);
        currentScope = prevScope;

        inTable->value = newClassMolde;
        newClassMolde->addReference(*inTable);

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_ClassBody(const std::shared_ptr<ClassBodyNode>& node) {
        for (auto& st : node->statements) {
            if (st->kind() == NodeType::StaticStatement)
                visit(Node::as<StaticStatementNode>(st)->statement);
        }

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_ConstructorDecl(const std::shared_ptr<ConstructorDeclNode>& node) {
        Symbol* retType = nullptr;

        auto paramTypes = getParamTypes(node->params);

        auto typeName = Symbol::constructFuncTypeName(retType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(retType, paramTypes);
        }

        auto funcValue = std::make_shared<FunctionValue>(typeOfFunc, node->params, node->body, currentScope);
        valueTable.addNewValue(funcValue);

        auto newFunctionSymbol = currentScope->addSymbol({typeOfFunc, "constructor", funcValue});
        funcValue->addReference(*newFunctionSymbol);

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_ConstructorCall(const std::shared_ptr<ConstructorCallNode>& node) {
        if (call_stack.size() >= MAX_CALL_DEPTH) {
            throw Exceptions::RecursionException("Callback depth exceeded.", current_line, current_col);
        }

        auto constr = currentScope->findSymbol(node->t.value);

        auto fVal = constr ? constr->value : nullptr;

        // If there's no constructor, just ignore it.
        if (!fVal) {
            constructorParams.clear();
            constructorParams.shrink_to_fit();
            return null;
        }

        if (fVal->kind() == ValueType::FunctionVal) {
            auto as_function_value = Value::as<FunctionValue>(fVal);
            SymbolTable funcScope = {"constructor-scope", {}, as_function_value->parentScope};
            auto calleeScope = currentScope;

            std::vector<std::shared_ptr<Node>> newDecls;
            std::vector< std::pair<Lexing::Token, std::shared_ptr<Value>> > initValues;

            for (int i = 0; i < as_function_value->params.size(); i++) {
                auto par = as_function_value->params[i];
                if (constructorParams.size() > i) {
                    Lexing::Token tok{Lexing::NOTHING, ""};
                    switch (par->kind()) {
                        case NodeType::VarDeclaration:
                            tok = Node::as<VarDeclarationNode>(par)->name;
                            break;
                        case NodeType::ListDeclaration:
                            tok = Node::as<ListDeclarationNode>(par)->name;
                            break;
                        default:
                            break;
                    }

                    if (tok.tp != Lexing::NOTHING) {
                        auto newValue = constructorParams[i];
                        if (newValue->is_copyable()) {
                            newValue = newValue->copy(valueTable);
                        }
                        initValues.emplace_back(tok, newValue);
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

            visit(as_function_value->body);
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

    std::shared_ptr<Value> Interpreter::visit_ClassInitializer(const std::shared_ptr<ClassInitializerNode>& node) {
        if (auto classInit = currentScope->findSymbol(node->name.value)) {
            auto classVal = Value::as<ClassValue>(classInit->value);

            SymbolTable instanceScope{"instance-" + node->name.value + "-scope", {}, classVal->parentScope};

            auto newInstance = std::make_shared<InstanceValue>(classInit, classVal, std::move(instanceScope));
            valueTable.addNewValue(newInstance);
            newInstance->important = true;

            std::vector<std::shared_ptr<Value>> newParams;
            newParams.reserve(node->params.size());
            for (const auto& v : node->params) newParams.push_back(visit(v));
            constructorParams = newParams;

            auto tempScope = currentScope;
            currentScope = &newInstance->ownScope;

            auto thisSym = currentScope->addSymbol({
               classInit,
               "this",
               newInstance
            });

            auto currentClass = classVal;
            auto node_as_class_body = Node::as<ClassBodyNode>(currentClass->body);
            auto myInstanceBody = std::make_shared<InstanceBodyNode>(node_as_class_body->statements);

            auto mainScope = new SymbolTable{
                "inherited-scope-0",
                {{"this", *thisSym}},
                classVal->parentScope
            };
            std::vector<std::shared_ptr<Node>> inheritedBody = {std::move(myInstanceBody)};
            std::vector<SymbolTable*> inheritedScopes = {mainScope};

            int level = 1;
            while (currentClass->type->tp != nullptr) {
                auto upperValue = currentClass->type->tp->value;

                currentClass = Value::as<ClassValue>(upperValue);
                auto inherBody = std::make_shared<InstanceBodyNode>(Node::as<BlockNode>(currentClass->body)->statements);

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
            auto initFuncCall = std::make_shared<ConstructorCallNode>(initID);

            visit(initFuncCall);

            currentScope = tempScope;

            newInstance->important = false;

            return newInstance;
        } else {
            throw Exceptions::NameException(
                node->name.value + " is not a valid constructor.",
                current_line,
                current_col
            );
        }
    }

    std::shared_ptr<Value> Interpreter::visit_InstanceBody(const std::shared_ptr<InstanceBodyNode>& node){
        for (auto& st : node->statements) {
            if (st->kind() != NodeType::StaticStatement)
                visit(st);
        }

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_MemberVar(const std::shared_ptr<MemberVarNode>& node) {
        auto instance = visit(node->inst);

        if (!instance || instance->kind() != ValueType::InstanceVal) {
            throw Exceptions::ValueException(
                "Invalid instance for Member Variable operator.",
                current_line,
                current_col
            );
        }

        auto as_instance_value = Value::as<InstanceValue>(instance);

        auto foundSymbol = as_instance_value->ownScope.findSymbol(node->name.value);
        if (foundSymbol) {
            if (foundSymbol->value) return foundSymbol->value;
        } else {
            throw Exceptions::NameException(
                "No member variable named '" + node->name.value + "' in the Instance.",
                current_line,
                current_col
            );
        }

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_StaticVar(const std::shared_ptr<StaticVarNode>& node) {
        auto instance = visit(node->inst);

        if (instance->kind() == ValueType::InstanceVal) {
            auto as_instance_value = Value::as<InstanceValue>(instance);
            auto classVal = as_instance_value->molde;

            auto foundSymbol = classVal->ownScope.findSymbol(node->name.value);
            if (foundSymbol) {
                if (foundSymbol->value) return foundSymbol->value;

                return null;
            } else {
                throw Exceptions::NameException(
                    "No static variable named '" + node->name.value + "' in Instance.",
                    current_line,
                    current_col
                );
            }
        } else if (instance->kind() == ValueType::ClassVal) {
            auto as_class_value = Value::as<ClassValue>(instance);
            auto foundSymbol = as_class_value->ownScope.findSymbol(node->name.value);
            if (foundSymbol) {
                if (foundSymbol->value) return foundSymbol->value;

                return null;
            } else {
                throw Exceptions::NameException(
                    "No static variable named '" + node->name.value + "' in class.",
                    current_line,
                    current_col
                );
            }
        } else if (instance->kind() == ValueType::ModuleVal) {
            auto as_module_value = Value::as<ModuleValue>(instance);
            auto sm = as_module_value->ownScope.findSymbol(node->name.value, false);
            if (sm) {
                if (sm->value)
                    return sm->value;
                return null;
            } else {
                throw Exceptions::NameException(
                    "No variable named '" + node->name.value + "' in module.",
                    current_line,
                    current_col
                );
            }
        } else if (instance->kind() == ValueType::EnumVal) {
            auto as_enum_value = Value::as<EnumValue>(instance);
            auto sm = as_enum_value->ownScope.findSymbol(node->name.value, false);
            if (sm) {
                if (sm->value)
                    return sm->value;
                return null;
            } else {
                throw Exceptions::NameException(
                    "'" + node->name.value + "' is not a variant in enum.",
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

    Symbol* Interpreter::getMemberVarSymbol(const std::shared_ptr<Node>& mem) {
        Symbol* varSym = nullptr;

        switch (mem->kind()) {
            case NodeType::Variable:
                varSym = currentScope->findSymbol(Node::as<VariableNode>(mem)->token.value);
                break;
            case NodeType::MemberVar:
            {
                auto as_member_node = Node::as<MemberVarNode>(mem);
                auto leftHandSym = getMemberVarSymbol(as_member_node->inst);

                if (leftHandSym && leftHandSym->value) {
                    auto theValue = leftHandSym->value;
                    if (theValue->kind() != ValueType::InstanceVal) {
                        throw Exceptions::ValueException(
                            "'" + leftHandSym->name + "' is not a valid instance.",
                            current_line,
                            current_col
                        );
                    }

                    auto as_instance_value = Value::as<InstanceValue>(theValue);

                    varSym = as_instance_value->ownScope.findSymbol(as_member_node->name.value);
                }
                break;
            }
            case NodeType::StaticVar:
            {
                auto as_static_var = Node::as<StaticVarNode>(mem);
                auto leftHandSym = getMemberVarSymbol(as_static_var->inst);

                if (leftHandSym && leftHandSym->value) {
                    auto theValue = leftHandSym->value;
                    if (theValue->kind() == ValueType::ModuleVal) {
                        varSym = Value::as<ModuleValue>(theValue)->ownScope.findSymbol(as_static_var->name.value, false);
                    } else if (theValue->kind() == ValueType::ClassVal) {
                        varSym = Value::as<ClassValue>(theValue)->ownScope.findSymbol(as_static_var->name.value);
                    } else if (theValue->kind() == ValueType::InstanceVal) {
                        auto classVal = Value::as<InstanceValue>(theValue)->molde;

                        varSym = classVal->ownScope.findSymbol(as_static_var->name.value);
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
            case NodeType::Index:
            {
                auto as_index_node = Node::as<IndexNode>(mem);
                auto visited_source = visit(as_index_node->val);

                if (visited_source->kind() == ValueType::ListVal) {
                    auto visited_indx = visit(as_index_node->expr);
                    if (visited_indx->type->name == "int") {
                        auto& as_list = Value::as<ListValue>(visited_source)->elements;
                        auto as_int = Value::as<NormalValue>(visited_indx)->as_int();
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

    std::shared_ptr<Value> Interpreter::eval(std::string code) {

        call_stack.push_back({"global", 1, 1});
        parser.set_text(std::move(code));

        auto statements = parser.program_content();

    //    try{
        currentScope = &replScope;

        auto result = null;
        for (const auto& node : statements) {
            result = visit(node);
        }

        currentScope = &globalTable;
    //    }
        call_stack.pop_back();

        return result;
    }

    void Interpreter::set_repl_last(std::shared_ptr<Value> v) {
        auto& last_value_symbol = replScope.symbols.at("_");
        // Note: This may have some mistakes if the value has no references.
        // But by the nature of the repl, there shouldn't be much problems
        // _ Should be treated as any value that was just created on the spot.
        last_value_symbol.value = std::move(v);
    }

    std::shared_ptr<Value> Interpreter::interpret_as_module(const std::string &path, const Lexing::Token& name) {
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

        auto file_module = std::make_shared<ModuleNode>(
            Lexing::Token(Lexing::STR, filename),
            body
        );

        call_stack.push_back({"module '" + full_path + "'", 1, 1});

        auto result = visit(file_module);

        call_stack.pop_back();
        return result;
    }

    std::vector<std::pair<Symbol, bool>> Interpreter::getParamTypes(const std::vector<std::shared_ptr<Node>>& params) {
        std::vector<std::pair<Symbol, bool>> ts;

        for (const auto& par : params) {
            switch (par->kind()) {
                case NodeType::VarDeclaration: {
                    auto as_var_declaration_node = Node::as<VarDeclarationNode>(par);
                    if (auto ft = currentScope->findSymbol(as_var_declaration_node->var_type.value)) {
                        auto is_not_optional = as_var_declaration_node->initial && as_var_declaration_node->initial->kind() != NodeType::NoOp;
                        ts.emplace_back(*ft, is_not_optional);
                    } else {
                        // TODO: Handle Error
                        // Error! Unknown type par.type.value
                        throw Exceptions::TypeException(
                                "Unknown type " + as_var_declaration_node->var_type.value + ".",
                                par->line_number,
                                par->column_number
                        );
                    }
                    break;
                }
                case NodeType::ListDeclaration: {// FIXME: List types are registered as their basetype and not as listtype
                    auto as_var_declaration_node = Node::as<ListDeclarationNode>(par);
                    if (auto ft = currentScope->findSymbol(as_var_declaration_node->var_type.value)) {
                        auto is_not_optional = as_var_declaration_node->initial && as_var_declaration_node->initial->kind() != NodeType::NoOp;
                        ts.emplace_back(*ft, is_not_optional);
                    } else {
                        throw Exceptions::TypeException(
                                "Unknown type " + as_var_declaration_node->var_type.value + ".",
                                par->line_number,
                                par->column_number
                        );
                    }
                    break;
                }
                default:
                    // Error! Expected variable declaration inside function parenthesis.
                    throw Exceptions::SyntaxException(
                            "Expected parameter declaration in function parenthesis",
                            par->line_number,
                            par->column_number
                    );
                    break;
            }
        }

        return ts;
    }
}