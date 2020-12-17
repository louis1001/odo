//
// Created by Luis Gonzalez on 3/20/20.
//

#include "Interpreter/Interpreter.h"
#include "Exceptions/exception.h"
#include "IO/io.h"
#include "utils.h"

#include "Translations/lang.h"

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
    throw Exceptions::OdoException(NOT_IMPL_EXCP + std::string(#__VA_ARGS__), current_line, current_col)

namespace Odo::Interpreting {
    using namespace Parsing;

    Interpreter::Interpreter(Parser p): parser(std::move(p)) {
        auto any_symbol = Symbol{.name=ANY_TP, .isType=true, .kind=SymbolType::PrimitiveType};

        std::unordered_map<std::string, Symbol> buildInTypes = {
            {ANY_TP, any_symbol}
        };
        globalTable = SymbolTable("global", buildInTypes);

        auto any_sym = &globalTable.symbols[ANY_TP];

        globalTable.symbols[INT_TP] = {.tp=any_sym, .name=INT_TP, .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols[DOUBLE_TP] = {.tp=any_sym, .name=DOUBLE_TP, .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols[STRING_TP] = {.tp=any_sym, .name=STRING_TP, .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols[BOOL_TP] = {.tp=any_sym, .name=BOOL_TP, .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols[POINTER_TP] = {.tp=any_sym, .name=POINTER_TP, .isType=true, .kind=SymbolType::PrimitiveType};
        globalTable.symbols[NULL_TP] = {.tp=any_sym, .name=NULL_TP, .isType=true, .kind=SymbolType::PrimitiveType};

        currentScope = &globalTable;

        replScope = SymbolTable("repl", {}, &globalTable);

        null = NormalValue::create(globalTable.findSymbol(NULL_TP), NULL_TK);

        globalTable.addSymbol({
            .tp = &globalTable.symbols[NULL_TP],
            .name = NULL_TK,
            .value = null
        });

        replScope.symbols["_"] = {.tp=any_sym, .name="_", .value=null, .isType=false, .kind=SymbolType::VarSymbol};

        returning = nullptr;

#ifdef DEBUG_FUNCTIONS
        add_native_function("valueAt", [&](auto values) {
            int a = values[0]->as_int();

            return &valueTable.value_map()[a];
        });
#endif

        add_native_function(PRINT_FN, [&](auto values) {
            for (const auto& v : values) {
                if (v)
                    std::cout << v->to_string();
            }
            // Might make printing slower... I don't know of a better way of doing this.
            std::cout.flush();
            return null;
        });

        add_native_function(PRINTLN_FN, [&](auto values) {
            for (const auto& v : values) {
                if (v)
                    std::cout << v->to_string();
            }
            std::cout << std::endl;
            return null;
        });

        add_native_function(MOVE_CRSR_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                auto nv = Value::as<NormalValue>(vals[0]);
                if (nv) {
                    std::cout << "\033[" << nv->as_int() << ";" << nv->as_int() << "H";
                }
            }

            return null;
        });

        add_native_function(FACTR_FN, [&](std::vector<std::shared_ptr<Value>> v){
            if (!v.empty() && v[0]->type->name == INT_TP) {
                int arg1 = Value::as<NormalValue>(v[0])->as_int();
                int result = 1;
                for(int i = 1; i <= arg1; i++) {
                    result = result * i;
                }
                return create_literal(result);
            }
            throw Exceptions::FunctionCallException(
                FACTR_REQ_INT_EXCP,
                current_line,
                current_col
            );
        });

        add_native_function(LENGTH_FN, [&](std::vector<std::shared_ptr<Value>> v){
            if (!v.empty()) {
                auto arg = v[0];
                if (arg->type->name == STRING_TP) {
                    size_t len = Value::as<NormalValue>(arg)->as_string().size();
                    return create_literal((int)len);
                } else if (arg->kind() == ValueType::ListVal) {
                    size_t len = Value::as<ListValue>(arg)->as_list_value().size();
                    return create_literal((int)len);
                }
            }
            throw Exceptions::FunctionCallException(
                LENGTH_REQ_ARGS_EXCP,
                current_line,
                current_col
            );
        });

        add_native_function(FROM_ASCII_FN, [&](std::vector<std::shared_ptr<Value>> vals){
            if (!vals.empty()) {
                int val = Value::as<NormalValue>(vals[0])->as_int();

                return create_literal(std::string(1, val));
            }

            return null;
        });

        add_native_function(TO_ASCII_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                char val = Value::as<NormalValue>(vals[0])->as_string()[0];

                return create_literal(static_cast<int>(val));
            }

            return null;
        });

        add_native_function(POW_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (vals.size() >= 2) {
                bool result_as_int = true;
                double a = 0, b = 0;

                auto normal_val_first = Value::as<NormalValue>(vals[0]);
                auto normal_val_second = Value::as<NormalValue>(vals[1]);

                if (normal_val_first && vals[0]->type->name == DOUBLE_TP) {
                    a = normal_val_first->as_double();
                    result_as_int = false;
                } else if (normal_val_first && vals[0]->type->name == INT_TP) {
                    a = normal_val_first->as_int();
                }

                if (normal_val_second->type->name == DOUBLE_TP) {
                    b = normal_val_second->as_double();
                    result_as_int = false;
                } else if (normal_val_second->type->name == INT_TP) {
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

        add_native_function(SQRT_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                double a = 0;

                auto normal_value = Value::as<NormalValue>(vals[0]);

                if (normal_value && normal_value->type->name == DOUBLE_TP) {
                    a = normal_value->as_double();
                } else if (normal_value && normal_value->type->name == INT_TP) {
                    a = normal_value->as_int();
                }

                return create_literal(sqrt(a));
            }
            return null;
        });
        add_native_function(SIN_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                double a = 0;

                auto normal_value = Value::as<NormalValue>(vals[0]);

                if (normal_value && normal_value->type->name == DOUBLE_TP) {
                    a = normal_value->as_double();
                } else if (normal_value && normal_value->type->name == INT_TP) {
                    a = normal_value->as_int();
                }

                return create_literal(sin(a));
            }
            return null;
        });
        add_native_function(COS_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                double a = 0;

                auto normal_value = Value::as<NormalValue>(vals[0]);

                if (normal_value && normal_value->type->name == DOUBLE_TP) {
                    a = normal_value->as_double();
                } else if (normal_value && normal_value->type->name == INT_TP) {
                    a = normal_value->as_int();
                }

                return create_literal(cos(a));
            }
            return null;
        });

        add_native_function(FLOOR_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (vals.size() == 1) {
                auto v1 = Value::as<NormalValue>(vals[0]);

                double v = 0;
                if (v1 && v1->type->name == DOUBLE_TP) {
                    v = v1->as_double();
                } else if (v1 && v1->type->name == INT_TP) {
                    v = v1->as_int();
                } else {
                    throw Exceptions::ValueException(FLOOR_ONLY_NUM_EXCP);
                }

                return create_literal(static_cast<int>(floor(v)));
            }

            return null;
        });

        add_native_function(TRUNC_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (vals.size() == 1) {
                auto v1 = Value::as<NormalValue>(vals[0]);
                double v = 0;
                if (v1 && v1->type->name == DOUBLE_TP) {
                    v = v1->as_double();
                } else if (v1 && v1->type->name == INT_TP) {
                    v = v1->as_int();
                } else {
                    throw Exceptions::ValueException(TRUNC_ONLY_NUM_EXCP);
                }

                return create_literal(static_cast<int>(trunc(v)));
            }

            return null;
        });

        add_native_function(ROUND_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            if (!vals.empty()) {
                auto v1 = Value::as<NormalValue>(vals[0]);
                double v = 0;
                if (v1 && v1->type->name == DOUBLE_TP) {
                    v = v1->as_double();
                } else if (v1 && v1->type->name == INT_TP) {
                    v = v1->as_int();
                } else {
                    throw Exceptions::ValueException(
                        ROUND_ONLY_NUM_EXCP,
                        current_line,
                        current_col
                    );
                }

                if (vals.size() == 1) {
                    return create_literal(static_cast<int>(round(v)));
                }

                auto v2 = Value::as<NormalValue>(vals[1]);
                if (!v2 || v2->type->name != INT_TP) {
                    throw Exceptions::ValueException(
                        ROUND_ONLY_NUM_EXCP,
                        current_line,
                        current_col
                    );
                }

                int rounding = v2->as_int();
                int decimal = static_cast<int>(pow(10, rounding));
                double result = round(v*decimal)/decimal;
                return create_literal(result);
            }

            return null;
        });

        add_native_function(READ_FN, [&](const std::vector<std::shared_ptr<Value>>& vals) {
            std::string result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }

            std::getline(std::cin, result);
            return create_literal(result);
        });

        add_native_function(READ_INT_FN, [&](const std::vector<std::shared_ptr<Value>>& vals) {
            int result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(result);
        });

        add_native_function(READ_DOUBLE_FN, [&](const std::vector<std::shared_ptr<Value>>& vals) {
            double result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(result);
        });

        add_native_function(RAND_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
            double min = 0.0;
            double max = 1.0;
            if (vals.size() == 1) {
                auto normal_value_first = Value::as<NormalValue>(vals[0]);
                if (vals[0]->type->name == DOUBLE_TP) {
                    max = normal_value_first->as_double();
                } else if (vals[0]->type->name == INT_TP) {
                    max = normal_value_first->as_int();
                }
            } else if (vals.size() >= 2) {
                auto normal_value_first = Value::as<NormalValue>(vals[0]);
                if (vals[0]->type->name == DOUBLE_TP) {
                    min =normal_value_first->as_double();
                } else if (vals[0]->type->name == INT_TP) {
                    min = normal_value_first->as_int();
                }

                auto normal_value_second = Value::as<NormalValue>(vals[1]);
                if (vals[1]->type->name == DOUBLE_TP) {
                    max = normal_value_second->as_double();
                } else if (vals[1]->type->name == INT_TP) {
                    max = normal_value_second->as_int();
                }
            }

            return create_literal(rand_double(min, max));
        });

        add_native_function(RAND_INT_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
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

        add_native_function(POP_FN, [&](std::vector<std::shared_ptr<Value>> vals) {
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

        add_native_function(TYPEOF_FN, [&](const auto& vals) {
            if (vals.size() == 1) {
                auto v = vals[0];
                if (v) return create_literal(v->type->name);
            }

            return null;
        });

        add_native_function(CLEAR_FN, [&](auto){std::cout << "\033[2J\033[1;1H"; return null;});

        add_native_function(WAIT_FN, [&](auto){ std::cin.get(); return null; });

        add_native_function(SLEEP_FN, [&](std::vector<std::shared_ptr<Value>> vals){
            if (!vals.empty()) {
                auto delay_time = Value::as<NormalValue>(vals[0])->as_int();

                std::this_thread::sleep_for(std::chrono::milliseconds(delay_time));
            }
            return null;
        });
    }

    Symbol* Interpreter::any_type() {
        return &globalTable.symbols[ANY_TP];
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

            if (lhs->type->name == INT_TP) {
                auto new_left = create_literal((double)left_numeric->as_int());

                result.first = std::move(new_left);
            }

            if (rhs->type->name == INT_TP) {
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
                    STATIC_ONLY_CLASS_EXCP,
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
        if (kind == DOUBLE_TP) {
            newValue = strtod(val.c_str(), nullptr);
        } else if (kind == INT_TP) {
            int a = (int) strtol(val.c_str(), nullptr, 10);
            newValue = a;
        } else if (kind == STRING_TP) {
            newValue = val;
        } else if (kind == BOOL_TP) {
            if (val != TRUE_TK && val != FALSE_TK){
                throw Exceptions::ValueException(
                    INVALID_BOOL_EXCP,
                    current_line,
                    current_col
                );
            }
            newValue = val == TRUE_TK;
        } else {
            return null;
        }

        // Handle errors in conversions are incorrect.

        auto normal_value = NormalValue::create(globalTable.findSymbol(kind), newValue);

        return normal_value;
    }

    std::shared_ptr<Value> Interpreter::create_literal_from_any(const std::any& val, const std::string &kind) {
        return NormalValue::create(globalTable.findSymbol(kind), val);
    }

    std::shared_ptr<Value> Interpreter::create_literal(std::string val) {
        return create_literal_from_any(val, STRING_TP);
    }

    std::shared_ptr<Value> Interpreter::create_literal(int val) {
        return create_literal_from_any(val, INT_TP);
    }

    std::shared_ptr<Value> Interpreter::create_literal(double val) {
        return create_literal_from_any(val, DOUBLE_TP);
    }

    std::shared_ptr<Value> Interpreter::create_literal(bool val) {
        return create_literal_from_any(val, BOOL_TP);
    }

    std::shared_ptr<Value> Interpreter::visit_Double(const std::shared_ptr<DoubleNode>& node) {
        return create_literal_from_string(node->token.value, DOUBLE_TP);
    }

    std::shared_ptr<Value> Interpreter::visit_Int(const std::shared_ptr<IntNode>& node) {
        return create_literal_from_string(node->token.value, INT_TP);
    }

    std::shared_ptr<Value> Interpreter::visit_Bool(const std::shared_ptr<BoolNode>& node) {
        return create_literal_from_string(node->token.value, BOOL_TP);
    }

    std::shared_ptr<Value> Interpreter::visit_Str(const std::shared_ptr<StrNode>& node) {
        return create_literal_from_string(node->token.value, STRING_TP);
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
        result->important = false;

        return result;
    }

    std::shared_ptr<Value> Interpreter::visit_TernaryOp(const std::shared_ptr<TernaryOpNode>& node) {
        auto val_cond = visit(node->cond);
        if (val_cond->type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    COND_TERN_MUST_BOOL_EXCP,
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
        if (val_cond->type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    COND_IF_MUST_BOOL_EXCP,
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
        if (val_cond->type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    COND_FOR_MUST_BOOL_EXCP,
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
        } else if (lst_value->type->name == STRING_TP) {
            auto iterator_decl = std::make_shared<VarDeclarationNode>(
                Lexing::Token(Lexing::TokenType::ID, STRING_TP),
                node->var,
                std::make_shared<NoOpNode>()
            );
            visit(iterator_decl);

            auto declared_iter = currentScope->findSymbol(node->var.value);
            declared_iter->value = create_literal(std::string(1, '\0'));

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

        } else {
            throw Exceptions::ValueException(
                    FOREACH_ONLY_LIST_STR_EXCP,
                    current_line,
                    current_col
            );
        }

        currentScope = forScope.getParent();

        return null;

    }

    std::shared_ptr<Value> Interpreter::visit_FoRange(const std::shared_ptr<FoRangeNode>& node){
        auto forScope = SymbolTable("forange:loop", {}, currentScope);
        currentScope = &forScope;

        int max_in_range = 0;
        auto first_visited = visit(node->first);
        if (!first_visited->is_numeric()) {
            throw Exceptions::ValueException(
                VAL_RANGE_NUM_EXCP,
                current_line,
                current_col
            );
        }

        if (first_visited->type->name == INT_TP)
            max_in_range = Value::as<NormalValue>(first_visited)->as_int();
        else if (first_visited->type->name == DOUBLE_TP)
            max_in_range = static_cast<int>(floor(Value::as<NormalValue>(first_visited)->as_double()));

        int min_in_range = 0;

        if (node->second && node->second->kind() != NodeType::NoOp) {
            min_in_range = max_in_range;

            auto second_visited = visit(node->second);
            if (!second_visited->is_numeric()) {
                throw Exceptions::ValueException(
                    VAL_RANGE_NUM_EXCP,
                    current_line,
                    current_col
                );
            }

            if (second_visited->type->name == INT_TP)
                max_in_range = Value::as<NormalValue>(second_visited)->as_int();
            else if (second_visited->type->name == DOUBLE_TP)
                max_in_range = static_cast<int>(floor(Value::as<NormalValue>(second_visited)->as_double()));
        }

        bool go_backwards = node->rev.tp != Lexing::NOTHING;
        bool use_iterator = node->var.tp != Lexing::NOTHING;

        Symbol* declared_iter {nullptr};
        std::shared_ptr<NormalValue> iter_as_normal;
        if (use_iterator) {
            std::shared_ptr<Node> iterator_decl = std::make_shared<VarDeclarationNode>(
                    Lexing::Token(Lexing::TokenType::ID, INT_TP),
                    node->var,
                    std::make_shared<NoOpNode>()
            );
            visit(iterator_decl);
            declared_iter = currentScope->findSymbol(node->var.value);
            declared_iter->value = create_literal(0);

            iter_as_normal = Value::as<NormalValue>(declared_iter->value);
        }

        for(int i = min_in_range; i < max_in_range; i++){
            auto actual_value = i;
            if (go_backwards) actual_value = min_in_range + max_in_range-1-i;

            if (use_iterator)
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

        currentScope = forScope.getParent();

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_While(const std::shared_ptr<WhileNode>& node) {
        auto whileScope = SymbolTable("while:loop", {}, currentScope);
        currentScope = &whileScope;

        auto val_cond = visit(node->cond);
        if (val_cond->type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    COND_WHILE_MUST_BOOL_EXCP,
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
                    VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
                    current_line,
                    current_col
            );
        } else {
            auto type_ = currentScope->findSymbol(node->var_type.value);

            if (type_ == nullptr) {
                throw Exceptions::NameException(
                        UNKWN_TYPE_EXCP + node->var_type.value + "'.",
                        current_line,
                        current_col
                );
            }

            Symbol newVar;
            std::shared_ptr<Value> valueReturn;

            if (node->initial && node->initial->kind() != NodeType::NoOp) {
                auto newValue = visit(node->initial);

                if (newValue->is_copyable()) {
                    newValue = newValue->copy();
                }

                if (type_->name == ANY_TP) {
                    type_ = newValue->type;
                } else {
                    if (type_->name == INT_TP && newValue->type->name == DOUBLE_TP) {
                        newValue = create_literal((int) Value::as<NormalValue>(newValue)->as_double());
                    } else if (type_->name == DOUBLE_TP && newValue->type->name == INT_TP) {
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

            currentScope->addSymbol(newVar);
            if (valueReturn != null) {
            }
            return valueReturn;
        }
        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_ListDeclaration(const std::shared_ptr<ListDeclarationNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
                    current_line,
                    current_col
            );
        }

        auto base_type = currentScope->findSymbol(node->var_type.value);
        if (!(base_type && base_type->isType)) {
            throw Exceptions::TypeException(
                    INVALID_TYPE_EXCP + node->var_type.value + "'.",
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


                if (newValue->is_copyable()) {
                    newValue = newValue->copy();
                }
            } else {
                if (newValue->is_copyable()) {
                    newValue = newValue->copy();
                }

                if (varSym->tp->name == ANY_TP) {
                    varSym->tp = newValue->type;
                } else {
                    if (varSym->tp->name == INT_TP && newValue->type->name == DOUBLE_TP) {
                        newValue = create_literal((int) Value::as<NormalValue>(newValue)->as_double());
                    } else if (varSym->tp->name == DOUBLE_TP && newValue->type->name == INT_TP) {
                        newValue = create_literal((double) Value::as<NormalValue>(newValue)->as_int());
                    }
                }
            }

            varSym->value = newValue;
        } else {
            throw Exceptions::NameException(
                    ASS_TO_UNKWN_VAR_EXCP,
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
                    VAR_CALLED_EXCP + node->token.value + NOT_DEFINED_EXCP,
                    current_line,
                    current_col
            );
        }
    }

    std::shared_ptr<Value> Interpreter::visit_Index(const std::shared_ptr<IndexNode>& node) {
        auto visited_val = visit(node->val);

        if (visited_val->type->name == STRING_TP) {
            auto str = Value::as<NormalValue>(visited_val)->as_string();

            auto visited_indx = visit(node->expr);
            if (visited_indx->type->name == INT_TP) {
                auto int_indx = Value::as<NormalValue>(visited_indx)->as_int();

                if (int_indx >= 0 && int_indx < str.size()) {
                    std::string result(1, str[int_indx]);
                    return create_literal_from_string(result, STRING_TP);
                } else if (int_indx < 0 && abs(int_indx) <= str.size()) {
                    size_t actual_indx = str.size() - int_indx;
                    std::string result(1, str[actual_indx]);
                    return create_literal_from_string(result, STRING_TP);
                } else {
                    throw Exceptions::ValueException(
                            INDX_STR_OB_EXCP,
                            current_line,
                            current_col
                    );
                }
            } else {
                throw Exceptions::TypeException(
                        STR_ONLY_INDX_NUM_EXCP,
                        current_line,
                        current_col
                );
            }
        } else if (visited_val->kind() == ValueType::ListVal){
            auto list_value = Value::as<ListValue>(visited_val)->as_list_value();
            auto visited_indx = visit(node->expr);
            if (visited_indx->type->name == INT_TP) {
                auto int_indx = Value::as<NormalValue>(visited_indx)->as_int();

                if (int_indx >= 0 && int_indx < list_value.size()) {
                    return list_value[int_indx];
                } else if (int_indx < 0 && abs(int_indx) <= list_value.size()) {
                    size_t actual_indx = list_value.size() - int_indx;
                    return list_value[actual_indx];
                } else {
                    throw Exceptions::ValueException(
                            INDX_LST_OB_EXCP,
                            current_line,
                            current_col
                    );
                }
            } else {
                throw Exceptions::TypeException(
                        LST_ONLY_INDX_NUM_EXCP,
                        current_line,
                        current_col
                );
            }
        } else {
            throw Exceptions::ValueException(
                    INDX_ONLY_LST_STR_EXCP,
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
                actual_value = visited_element->copy();
            } else {
                actual_value = visited_element;
            }

            auto el_symbol = Symbol{
                type_of_el,
                "list_element",
                actual_value
            };

            list_syms.push_back(el_symbol);
        }

        Symbol* list_type;
        if (list_t) {
            list_type = list_t;
        } else {
            list_type = globalTable.addListType(any_type());
        }

        auto new_list_value = ListValue::create(list_type, std::move(list_syms));

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
                                val = val->copy();
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            new_elements.push_back(new_symbol);
                        }
                        for (const auto &el : right_as_list->elements) {
                            auto val = el.value;

                            if (val->is_copyable()) {
                                val = val->copy();
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            new_elements.push_back(new_symbol);
                        }

                        new_list = ListValue::create(leftVisited->type, std::move(new_elements));

                        return new_list;
                    } else {
                        std::vector<Symbol> new_elements;

                        for (const auto &el : left_as_list->elements) {
                            auto val = el.value;

                            if (val->is_copyable()) {
                                val = val->copy();
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            new_elements.push_back(new_symbol);
                        }

                        auto val = rightVisited;
                        if (val->is_copyable()) {
                            val = val->copy();
                        }

                        Symbol new_symbol = {val->type, "list_element", val};
                        new_elements.push_back(new_symbol);

                        new_list = std::make_shared<ListValue>(
                                globalTable.addListType(val->type),
                                std::move(new_elements)
                        );
                        return new_list;
                    }
                } else if (leftVisited->type->name == STRING_TP) {
                    return create_literal(left_as_normal->as_string() + rightVisited->to_string());
                } else if (rightVisited->type->name == STRING_TP) {
                    return create_literal(leftVisited->to_string() + right_as_normal->as_string());
                } else if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() + right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() + right_as_normal->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            ADD_ONLY_SAME_TP_EXCP,
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
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() - right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() - right_as_normal->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            SUB_ONLY_SAME_TP_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::MUL: {
                auto left_as_normal = Value::as<NormalValue>(leftVisited);
                auto right_as_normal = Value::as<NormalValue>(rightVisited);
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() * right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() * right_as_normal->as_double();
                        return create_literal(result);
                    }
                } else if (leftVisited->kind() == ValueType::ListVal && rightVisited->type->name == INT_TP) {
                    int right_as_int = right_as_normal->as_int();
                    std::vector<Symbol> new_elements;

                    for (int i = 0; i < right_as_int; i++) {
                        for (const auto &el : Value::as<ListValue>(leftVisited)->elements) {
                            auto val = el.value;

                            if (val->is_copyable()) {
                                val = val->copy();
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            new_elements.push_back(new_symbol);
                        }
                    }

                    auto new_list = ListValue::create(leftVisited->type, std::move(new_elements));

                    return new_list;
                } else if (leftVisited->type->name == STRING_TP && rightVisited->type->name == INT_TP) {
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
                            MUL_ONLY_SAME_TP_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            }
            case Lexing::DIV: {
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == DOUBLE_TP) {
                    auto result = Value::as<NormalValue>(leftVisited)->as_double() / Value::as<NormalValue>(rightVisited)->as_double();
                    return create_literal(result);
                } else {
                    throw Exceptions::TypeException(
                            DIV_ONLY_DOB_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            }
            case Lexing::MOD:
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == INT_TP) {
                    auto result = Value::as<NormalValue>(leftVisited)->as_int() % Value::as<NormalValue>(rightVisited)->as_int();
                    return create_literal(result);
                }else {
                    throw Exceptions::TypeException(
                            MOD_ONLY_INT_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::POW:
                if (leftVisited->type == rightVisited->type) {
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == INT_TP) {
                        auto result = powl(left_as_normal->as_int(), right_as_normal->as_int());
                        return create_literal((double)result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = powl(left_as_normal->as_double(), right_as_normal->as_double());
                        return create_literal((double)result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            POW_ONLY_SAME_TP_EXCP,
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
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() == right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() == right_as_normal->as_double();
                        return create_literal(result);
                    } else if (leftVisited->type->name == BOOL_TP) {
                        auto result = left_as_normal->as_bool() == right_as_normal->as_bool();
                        return create_literal(result);
                    } else if (leftVisited->type->name == STRING_TP) {
                        auto result = left_as_normal->as_string() == right_as_normal->as_string();
                        return create_literal(result);
                    } else {
                        return create_literal(false);
                    }
                } else if (leftVisited->type == null->type || rightVisited->type == null->type) {
                    return create_literal(false);
                } else {
                    throw Exceptions::TypeException(
                            COM_ONLY_SAME_TP_EXCP,
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
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() != right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() != right_as_normal->as_double();
                        return create_literal(result);
                    } else if (leftVisited->type->name == BOOL_TP) {
                        auto result = left_as_normal->as_bool() != right_as_normal->as_bool();
                        return create_literal(result);
                    } else if (leftVisited->type->name == STRING_TP) {
                        auto result = left_as_normal->as_string() != right_as_normal->as_string();
                        return create_literal(result);
                    } else {
                        return create_literal(true);
                    }
                } else if (leftVisited->type == null->type || rightVisited->type == null->type) {
                    return create_literal(true);
                } else {
                    throw Exceptions::TypeException(
                            COM_ONLY_SAME_TP_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::LT:
                if (leftVisited->type == rightVisited->type) {
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() < right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() < right_as_normal->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            COM_ONLY_SAME_TP_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::GT:
                if (leftVisited->type == rightVisited->type) {
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() > right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() > right_as_normal->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            COM_ONLY_SAME_TP_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::LET:
                if (leftVisited->type == rightVisited->type) {
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() <= right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() <= right_as_normal->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            COM_ONLY_SAME_TP_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::GET:
                if (leftVisited->type == rightVisited->type) {
                    auto left_as_normal = Value::as<NormalValue>(leftVisited);
                    auto right_as_normal = Value::as<NormalValue>(rightVisited);
                    if (leftVisited->type->name == INT_TP) {
                        auto result = left_as_normal->as_int() >= right_as_normal->as_int();
                        return create_literal(result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = left_as_normal->as_double() >= right_as_normal->as_double();
                        return create_literal(result);
                    }
                } else {
                    throw Exceptions::TypeException(
                            COM_ONLY_SAME_TP_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::AND:
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == BOOL_TP) {
                    auto result = Value::as<NormalValue>(leftVisited)->as_bool() && Value::as<NormalValue>(rightVisited)->as_bool();
                    return create_literal(result);
                }else {
                    throw Exceptions::TypeException(
                            LOG_ONLY_BOOL_EXCP,
                            current_line,
                            current_col
                    );
                }
                break;
            case Lexing::OR:
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == BOOL_TP) {
                    auto result = Value::as<NormalValue>(leftVisited)->as_bool() || Value::as<NormalValue>(rightVisited)->as_bool();
                    return create_literal(result);
                }else {
                    throw Exceptions::TypeException(
                            LOG_ONLY_BOOL_EXCP,
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
                if (result->type->name == INT_TP) {
                    auto actual_value = result_as_normal->as_int();

                    result_as_normal->val = actual_value * -1;
                    return result;
                }

                if (result->type->name == DOUBLE_TP) {
                    auto actual_value = result_as_normal->as_double();

                    result_as_normal->val = actual_value * -1;
                    return result;
                }

                throw Exceptions::TypeException(
                        UNA_ONLY_NUM_EXCP,
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

        auto moduleValue = ModuleValue::create(null->type, module_scope);
        moduleValue->important = true;

        auto temp = currentScope;
        currentScope = &moduleValue->ownScope;
        for (const auto& st : node->statements) {
            visit(st);
        }

        currentScope = temp;
        moduleValue->important = false;

        currentScope->addSymbol({nullptr, node->name.value, moduleValue});

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

        auto funcValue = FunctionValue::create(typeOfFunc, node->params, node->body, currentScope);

        return funcValue;
    }

    std::shared_ptr<Value> Interpreter::visit_FuncDecl(const std::shared_ptr<FuncDeclNode>& node){

        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
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

        auto funcValue = FunctionValue::create(typeOfFunc, node->params, node->body, currentScope, node->name.value);

        currentScope->addSymbol({
            .tp=typeOfFunc,
            .name=node->name.value,
            .value=funcValue,
            .kind=SymbolType::FunctionSymbol
        });


        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_FuncCall(const std::shared_ptr<FuncCallNode>& node) {
        if (call_stack.size() >= MAX_CALL_DEPTH) {
            throw Exceptions::RecursionException(CALL_DEPTH_EXC_EXCP, current_line, current_col);
        }

        if (node->fname.tp != Lexing::NOTHING){
            auto found_in_natives = native_functions.find(node->fname.value);

            if (found_in_natives != native_functions.end()) {
                auto num_args = node->args.size();
                std::vector<std::shared_ptr<Value>> arguments_visited;

                std::vector<bool> was_important{};
                for(int i = 0; i < num_args; i++){
                    auto& arg = node->args[i];
                    auto v = visit(arg);
                    was_important.push_back(v->important);
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

        auto fVal = visit(node->expr);
        if (fVal->kind() == ValueType::NativeFunctionVal) {
            return Value::as<NativeFunctionValue>(fVal)->visit(*this, {});
        } else if (fVal->kind() == ValueType::FunctionVal) {
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
                                newValue = newValue->copy();
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

            call_stack.push_back({as_function_value->name, current_line, current_col});

            for (int i = 0; i < newDecls.size(); i++) {
                visit(newDecls[i]);

                if (i < initValues.size()) {
                    auto newVar = currentScope->findSymbol(initValues[i].first.value);
                    newVar->value = initValues[i].second;
                }
            }

            auto body_as_ast = as_function_value->body;

            auto result = visit(body_as_ast);
            currentScope = calleeScope;

            result->important = false;
            call_stack.pop_back();
            return result;
        }

        throw Exceptions::ValueException(VAL_NOT_FUNC_EXCP, current_line, current_col);
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
                    VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
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

            auto variant_value = EnumVarValue::create(enumInTable, variant_name);

            enum_variant_scope.addSymbol({
                enumInTable,
                variant_name,
                variant_value
            });

        }

        auto newValue = EnumValue::create(null->type, enum_variant_scope);
        enumInTable->value = newValue;

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_Class(const std::shared_ptr<ClassNode>& node) {
        Symbol* typeSym = nullptr;

        if (node->ty.tp != Lexing::NOTHING) {
            auto sym = currentScope->findSymbol(node->ty.value);
            if (!sym || !sym->isType) {
                throw Exceptions::TypeException(
                        CLASS_MUST_INH_TYPE_EXCP + node->name.value + IS_INVALID_EXCP,
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
        auto newClassMolde = ClassValue::create(inTable, classScope, currentScope, node->body);

        auto prevScope = currentScope;
        currentScope = &newClassMolde->ownScope;

        visit(node->body);
        currentScope = prevScope;

        inTable->value = newClassMolde;

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

        auto funcValue = FunctionValue::create(typeOfFunc, node->params, node->body, currentScope);

        currentScope->addSymbol({typeOfFunc, "constructor", funcValue});

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_ConstructorCall(const std::shared_ptr<ConstructorCallNode>& node) {
        if (call_stack.size() >= MAX_CALL_DEPTH) {
            throw Exceptions::RecursionException(CALL_DEPTH_EXC_EXCP, current_line, current_col);
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
                            newValue = newValue->copy();
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
                }
            }

            visit(as_function_value->body);
            currentScope = calleeScope;
            call_stack.pop_back();
        } else {
            // Error.
            throw Exceptions::ValueException(
                    INVALID_CONS_EXCP,
                current_line,
                current_col
            );
        }
        constructorParams.clear();
        constructorParams.shrink_to_fit();

        return null;
    }

    std::shared_ptr<Value> Interpreter::visit_ClassInitializer(const std::shared_ptr<ClassInitializerNode>& node) {
        if (auto classInit = currentScope->findSymbol(node->name.value);
            auto classVal = Value::as<ClassValue>(classInit->value)) {

            SymbolTable instanceScope{"instance-" + node->name.value + "-scope", {}, classVal->parentScope};

            auto newInstance = InstanceValue::create(classInit, classVal, instanceScope);
            Node::as<FunctionValue>(node);
            newInstance->important = true;

            std::vector<std::shared_ptr<Value>> newParams;
            newParams.reserve(node->params.size());
            for (const auto& v : node->params) newParams.push_back(visit(v));
            constructorParams = newParams;

            auto tempScope = currentScope;
            currentScope = &newInstance->ownScope;

            auto thisSym = currentScope->addSymbol({
               classInit,
               THIS_VAR,
               newInstance
            });

            auto currentClass = classVal;
            auto node_as_class_body = Node::as<ClassBodyNode>(currentClass->body);
            auto myInstanceBody = std::make_shared<InstanceBodyNode>(node_as_class_body->statements);

            auto mainScope = new SymbolTable{
                "inherited-scope-0",
                {{THIS_VAR, *thisSym}},
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
                    {{THIS_VAR, *thisSym}},
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
                node->name.value + NOT_VALID_CONS_EXCP,
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
                INVALID_INS_MEM_OP_EXCP,
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
                   NO_MEM_CALLED_EXCP + node->name.value + IN_THE_INST_EXCP,
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
                    NO_STATIC_CALLED_EXCP + node->name.value + IN_THE_INST_EXCP,
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
                    NO_STATIC_CALLED_EXCP + node->name.value + IN_CLASS_EXCP,
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
                        NO_VAR_NAMED_EXCP + node->name.value + IN_MODULE_EXCP,
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
                    "'" + node->name.value + NOT_VARIANT_IN_ENUM_EXCP,
                    current_line,
                    current_col
                );
            }
        } else {
            throw Exceptions::ValueException(
                CANNOT_READ_STATIC_EXCP,
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
                            "'" + leftHandSym->name + NOT_VALID_INST_EXCP,
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
                                INVALID_STATIC_OP_EXCP,
                                current_line,
                                current_col
                        );
                    }
                } else {
                    throw Exceptions::NameException(
                            UNKWN_VAL_IN_STATIC_EXCP,
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
                    if (visited_indx->type->name == INT_TP) {
                        auto& as_list = Value::as<ListValue>(visited_source)->elements;
                        auto as_int = Value::as<NormalValue>(visited_indx)->as_int();
                        return &as_list[as_int];
                    } else {
                        throw Exceptions::TypeException(
                            LST_ONLY_INDX_NUM_EXCP,
                            current_line,
                            current_col
                        );
                    }
                } else {
                    throw Exceptions::ValueException(
                            ASS_TO_INVALID_INDX_EXCP,
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
                SYM_CALLED_EXCP + filename + ALR_EXISTS_IN_SCOPE_EXCP,
                current_line,
                current_col
            );
        }

        std::string code;
        try {
            code = io::read_file(full_path);
        } catch (Exceptions::IOException&) {
            std::string msg = CANNOT_IMPORT_MODULE_EXCP + full_path + "'.";
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
                                UNKWN_TYPE_EXCP + as_var_declaration_node->var_type.value + "'.",
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
                                UNKWN_TYPE_EXCP + as_var_declaration_node->var_type.value + "'.",
                                par->line_number,
                                par->column_number
                        );
                    }
                    break;
                }
                default:
                    // Error! Expected variable declaration inside function parenthesis.
                    throw Exceptions::SyntaxException(
                            EXPCT_DECL_IN_PAR_EXCP,
                            par->line_number,
                            par->column_number
                    );
                    break;
            }
        }

        return ts;
    }
}