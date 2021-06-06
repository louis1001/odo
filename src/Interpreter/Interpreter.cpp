//
// Created by Luis Gonzalez on 3/20/20.
//

#include "Interpreter/Interpreter.h"
#include "Exceptions/exception.h"
#include "IO/io.h"
#include "utils.h"
#include "SemAnalyzer/SemanticAnalyzer.h"

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
#include "Parser/AST/DefineNode.h"
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

namespace Odo::Interpreting {
    using namespace Parsing;

    Interpreter::Interpreter(Parser p): parser(std::move(p)) {
        auto any_symbol = Symbol{.name=ANY_TP, .isType=true, .kind=SymbolType::PrimitiveType};

        std::unordered_map<std::string, Symbol> buildInTypes {
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

        int_type = &globalTable.symbols[INT_TP];
        double_type = &globalTable.symbols[DOUBLE_TP];
        string_type = &globalTable.symbols[STRING_TP];
        bool_type = &globalTable.symbols[BOOL_TP];

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
        returning_native = nullptr;

        analyzer = std::make_shared<Semantics::SemanticAnalyzer>(*this);

#ifdef DEBUG_FUNCTIONS
        add_native_function("valueAt", [&](auto values) {
            int a = values[0]->as_int();

            return &valueTable.value_map()[a];
        });
#endif

        add_native_function(WRITE_FN, [&](auto values) {
            for (const auto& v : values) {
                if (v)
                    std::cout << v->to_string();
            }
            // Might make printing slower... I don't know of a better way of doing this.
            std::cout.flush();
            return null;
        });

        add_native_function(WRITELN_FN, [&](auto values) {
            for (const auto& v : values) {
                if (v)
                    std::cout << v->to_string();
            }
            std::cout << std::endl;
            return null;
        });

        add_native_function(MOVE_CRSR_FN, [&](std::vector<value_t> vals) {
            if (!vals.empty()) {
                auto nv = Value::as<NormalValue>(vals[0]);
                if (nv) {
                    std::cout << "\033[" << nv->as_int() << ";" << nv->as_int() << "H";
                }
            }

            return null;
        });

        add_native_function(FACTR_FN, [&](std::vector<value_t> v){
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

        add_native_function(LENGTH_FN, [&](std::vector<value_t> v){
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

        add_native_function(FROM_ASCII_FN, [&](std::vector<value_t> vals){
            if (!vals.empty()) {
                int val = Value::as<NormalValue>(vals[0])->as_int();

                return create_literal(std::string(1, val));
            }

            return null;
        });

        add_native_function(TO_ASCII_FN, [&](std::vector<value_t> vals) {
            if (!vals.empty()) {
                char val = Value::as<NormalValue>(vals[0])->as_string()[0];

                return create_literal(static_cast<int>(val));
            }

            return null;
        });

        add_native_function(POW_FN, [&](std::vector<value_t> vals) {
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

        add_native_function(SQRT_FN, [&](std::vector<value_t> vals) {
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
        add_native_function(SIN_FN, [&](std::vector<value_t> vals) {
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
        add_native_function(COS_FN, [&](std::vector<value_t> vals) {
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

        add_native_function(FLOOR_FN, [&](std::vector<value_t> vals) {
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

        add_native_function(TRUNC_FN, [&](std::vector<value_t> vals) {
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

        add_native_function(ROUND_FN, [&](std::vector<value_t> vals) {
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

        add_native_function(READ_FN, [&](const std::vector<value_t>& vals) {
            std::string result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }

            std::getline(std::cin, result);
            return create_literal(result);
        });

        add_native_function(READ_INT_FN, [&](const std::vector<value_t>& vals) {
            int result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(result);
        });

        add_native_function(READ_DOUBLE_FN, [&](const std::vector<value_t>& vals) {
            double result;
            for (const auto& v : vals) {
                std::cout << v->to_string();
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(result);
        });

        add_native_function(RAND_FN, [&](std::vector<value_t> vals) {
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

        add_native_function(RAND_INT_FN, [&](std::vector<value_t> vals) {
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

        add_native_function(POP_FN, [&](std::vector<value_t> vals) {
            if (vals.size() == 1) {
                auto& lst = vals[0];
                if (lst->kind() == ValueType::ListVal) {
                    std::vector<Symbol>& syms = Value::as<ListValue>(lst)->elements;

                    if (!syms.empty()) {
                        Symbol& result = syms.back();
                        returning_native = result.value;
                        syms.pop_back();
                        return result.value ? result.value : null;
                    }
                }
            }

            return null;
        });

        add_native_function(PUSH_FN, [&](std::vector<value_t> vals) {
            if (vals.size() == 2) {
                auto& lst = vals[0];
                if (lst->kind() == ValueType::ListVal) {
                    auto to_push = vals[1];
                    std::vector<Symbol>& syms = Value::as<ListValue>(lst)->elements;

                    auto actual_value = to_push;
                    if (to_push->is_copyable()) actual_value = actual_value->copy();
                    Symbol newSym { actual_value->type, "list_element", actual_value };

                    syms.push_back(newSym);
                    return actual_value;
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

        add_function(CLEAR_FN, {}, nullptr, [](auto){std::cout << "\033[2J\033[1;1H"; return 0;});
        add_function(WAIT_FN, {}, nullptr,[](auto){ std::cin.get(); return 0; });

        add_function(SLEEP_FN, {{&globalTable.symbols.at(INT_TP), false}}, nullptr, [](auto vals){
            auto delay_time = std::any_cast<int>(vals[0]);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_time));

            return 0;
        });

        add_native_function(READ_FILE_FN, [&](const auto& vals){
            if (!vals.empty()) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                try {
                    auto contents = io::read_file(path);
                    return create_literal(contents);
                } catch (Exceptions::IOException&) {
                    throw Exceptions::FileException(COULD_NOT_READ_EXCP + path + MAY_NOT_EXIST_EXCP);
                }
            }
            return null;
        });

        add_native_function(TO_ABS_PATH_FN, [&](const auto& vals){
            if (!vals.empty()) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                return create_literal(io::to_absolute_path(path));
            }
            return null;
        });

        add_native_function(GET_CWD_FN, [&](const auto& vals){
            return create_literal(io::get_cwd());
        });

        add_native_function(PATH_EXISTS_FN, [&](const auto& vals){
            if (!vals.empty()) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                return create_literal(io::path_exists(path));
            }
            return null;
        });
        add_native_function(IS_DIR_FN, [&](const auto& vals){
            if (!vals.empty()) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                return create_literal(io::is_dir(path));
            }
            return null;
        });

        add_native_function(IS_FILE_FN, [&](const auto& vals){
            if (!vals.empty()) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                return create_literal(io::is_file(path));
            }
            return null;
        });

        add_native_function(LIST_DIR_FN, [&](const auto& vals){
            if (!vals.empty()) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                auto result = io::list_dir(path);

                std::vector<Symbol> lst_syms;
                for(const auto& f : result) {
                    value_t val = create_literal(f);
                    lst_syms.push_back({
                        val->type,
                        "list_element",
                        val
                    });
                }

                value_t list_value = ListValue::create(globalTable.findSymbol(STRING_TP), std::move(lst_syms));
                return list_value;
            }
            return null;
        });

        add_native_function(CREATE_FILE_FN, [&](const auto& vals){
            if (!vals.empty()) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                io::create_file(path);
            }
            return null;
        });

        add_native_function(WRITE_TO_FILE_FN, [&](const auto& vals){
            if (vals.size() >= 2) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                auto content = Value::as<NormalValue>(vals[1])->as_string();
                try {
                    io::write_to_file(path, content);
                } catch (Exceptions::IOException&) {
                    throw Exceptions::FileException(COULD_NOT_WRITE_EXCP + path + FOL_MAY_NOT_EXIST_EXCP);
                }
            }
            return null;
        });

        add_native_function(APPEND_TO_FILE_FN, [&](const auto& vals){
            if (vals.size() >= 2) {
                auto path = Value::as<NormalValue>(vals[0])->as_string();
                auto content = Value::as<NormalValue>(vals[1])->as_string();
                try {
                    io::append_to_file(path, content);
                } catch (Exceptions::IOException&) {
                    throw Exceptions::FileException(COULD_NOT_WRITE_EXCP + path + FOL_MAY_NOT_EXIST_EXCP);
                }
            }
            return null;
        });
    }

    Symbol* Interpreter::any_type() {
        return &globalTable.symbols[ANY_TP];
    }

    void Interpreter::add_function(
        const std::string& name,
        const std::vector<std::pair<Symbol*, bool>>& args,
        Symbol* ret,
        const std::function<std::any(std::vector<std::any>)>& callback
    ) {
        auto function_type = globalTable.addFuncType(ret, args);

        auto function_symbol = globalTable.addSymbol({
            .tp=function_type,
            .name=name,
            .kind=Interpreting::SymbolType::FunctionSymbol
        });

        analyzer->get_function_context_map().insert({function_type, args});

        auto func_value = Interpreting::NativeFunctionValue::create(function_type, args, callback);
        function_symbol->value = func_value;
        function_symbol->is_initialized = true;
    }

    void Interpreter::add_function(
            const std::string &name,
            const std::vector<std::pair<Symbol*, bool>>& args,
            Symbol *ret,
            const std::function<value_t(
                    std::vector<value_t>)> &callback
    ) {
        auto function_type = globalTable.addFuncType(ret, args);

        auto function_symbol = globalTable.addSymbol({
            .tp=function_type,
            .name=name,
            .kind=Interpreting::SymbolType::FunctionSymbol
        });

        analyzer->get_function_context_map().insert({function_type, args});

        auto func_value = Interpreting::NativeFunctionValue::create(function_type, args, callback);
        function_symbol->value = func_value;
        function_symbol->is_initialized = true;
    }

    std::pair<value_t, value_t>
    Interpreter::coerce_type(const value_t& lhs, const value_t& rhs) {
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

    value_t Interpreter::visit(const std::shared_ptr<Node>& node) {
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

            case NodeType::Define:
                return visit_Define(Node::as<DefineNode>(node));

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

    void Interpreter::add_function(const std::string& name, const std::function<void()> &callback) {
        add_function(name, {}, nullptr, [callback](auto){ callback(); return 0; });
    }

    void Interpreter::add_module(std::shared_ptr<Modules::NativeModule> moduleValue) {
        auto mod_table = moduleValue->ownScope;
        auto modSym = currentScope->addSymbol({
            nullptr,
            moduleValue->module_name(),
            std::move(moduleValue),
            false,
            SymbolType::ModuleSymbol,
            true
        });

        analyzer->add_semantic_context(modSym, mod_table);
    }

    value_t Interpreter::create_literal_from_string(std::string val, const std::string& kind) {
        if (kind == DOUBLE_TP) {
            auto newValue = strtod(val.c_str(), nullptr);
            return create_literal(newValue);
        } else if (kind == INT_TP) {
            int a = (int) strtol(val.c_str(), nullptr, 10);
            return create_literal(a);
        } else if (kind == STRING_TP) {
            return create_literal(val);
        } else if (kind == BOOL_TP) {
            if (val != TRUE_TK && val != FALSE_TK){
                throw Exceptions::ValueException(
                    INVALID_BOOL_EXCP,
                    current_line,
                    current_col
                );
            }
            return create_literal(val == TRUE_TK);
        } else {
            return null;
        }

        // Handle errors in conversions are incorrect.
    }

    value_t Interpreter::create_literal(std::string val) {
        return NormalValue::create(string_type, val);
    }

    value_t Interpreter::create_literal(int val) {
        return NormalValue::create(int_type, val);
    }

    value_t Interpreter::create_literal(double val) {
        return NormalValue::create(double_type, val);
    }

    value_t Interpreter::create_literal(bool val) {
        return NormalValue::create(bool_type, val);
    }

    value_t Interpreter::visit_Double(const std::shared_ptr<DoubleNode>& node) {
        return create_literal_from_string(node->token.value, DOUBLE_TP);
    }

    value_t Interpreter::visit_Int(const std::shared_ptr<IntNode>& node) {
        return create_literal_from_string(node->token.value, INT_TP);
    }

    value_t Interpreter::visit_Bool(const std::shared_ptr<BoolNode>& node) {
        return create_literal_from_string(node->token.value, BOOL_TP);
    }

    value_t Interpreter::visit_Str(const std::shared_ptr<StrNode>& node) {
        return create_literal_from_string(node->token.value, STRING_TP);
    }

    value_t Interpreter::visit_Block(const std::shared_ptr<BlockNode>& node) {
        auto blockScope = SymbolTable("block_scope", {}, currentScope);
        currentScope = &blockScope;

        for (const auto& st : node->statements) {
            visit(st);
            if (breaking || continuing || returning) {
                break;
            }
        }

        currentScope = blockScope.getParent();

        return null;
    }

    value_t Interpreter::visit_TernaryOp(const std::shared_ptr<TernaryOpNode>& node) {
        auto val_cond = visit(node->cond);

        bool real_condition = Value::as<NormalValue>(val_cond)->as_bool();

        if (real_condition) {
            return visit(node->trueb);
        } else {
            return visit(node->falseb);
        }
        return nullptr;
    }

    value_t Interpreter::visit_If(const std::shared_ptr<IfNode>& node) {
        auto val_cond = visit(node->cond);

        bool real_condition = Value::as<NormalValue>(val_cond)->as_bool();

        if (real_condition) {
            return visit(node->trueb);
        } else if (node->falseb) {
            return visit(node->falseb);
        }
        return null;
    }

    value_t Interpreter::visit_For(const std::shared_ptr<ForNode>& node) {
        auto forScope = SymbolTable("for:loop", {}, currentScope);
        currentScope = &forScope;

        visit(node->ini);

        auto val_cond = visit(node->cond);

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

    value_t Interpreter::visit_ForEach(const std::shared_ptr<ForEachNode>& node) {
        auto forScope = SymbolTable("foreach:loop", {}, currentScope);
        currentScope = &forScope;

        auto lst_value = visit(node->lst);
        if (lst_value->kind() == ValueType::ListVal) {
            std::shared_ptr<Node> iterator_decl;
            auto empty_initial = NoOpNode::create();
            auto element_tp =
                    VariableNode::create(
                    Lexing::Token(Lexing::TokenType::ID, lst_value->type->name));
            if (lst_value->type->tp && lst_value->type->tp->kind == SymbolType::ListType) {
                iterator_decl = ListDeclarationNode::create(
                    std::move(element_tp),
                    node->var,
                    1, // 1 because the element_tp should already exist as multidimensional if needed.
                    std::move(empty_initial)
                );
            } else {
                iterator_decl = VarDeclarationNode::create(std::move(element_tp), node->var, std::move(empty_initial));
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
            auto iterator_decl = VarDeclarationNode::create(
                VariableNode::create(Lexing::Token(Lexing::TokenType::ID, STRING_TP)),
                node->var,
                NoOpNode::create()
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
        }

        currentScope = forScope.getParent();

        return null;

    }

    value_t Interpreter::visit_FoRange(const std::shared_ptr<FoRangeNode>& node){
        auto forScope = SymbolTable("forange:loop", {}, currentScope);
        currentScope = &forScope;

        int max_in_range = 0;
        auto first_visited = visit(node->first);

        if (first_visited->type->name == INT_TP)
            max_in_range = Value::as<NormalValue>(first_visited)->as_int();
        else if (first_visited->type->name == DOUBLE_TP)
            max_in_range = static_cast<int>(floor(Value::as<NormalValue>(first_visited)->as_double()));

        int min_in_range = 0;

        if (node->second && node->second->kind() != NodeType::NoOp) {
            min_in_range = max_in_range;

            auto second_visited = visit(node->second);

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
            std::shared_ptr<Node> iterator_decl = VarDeclarationNode::create(
                    VariableNode::create(Lexing::Token(Lexing::TokenType::ID, INT_TP)),
                    node->var,
                    NoOpNode::create()
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

    value_t Interpreter::visit_While(const std::shared_ptr<WhileNode>& node) {
        auto whileScope = SymbolTable("while:loop", {}, currentScope);
        currentScope = &whileScope;

        auto val_cond = visit(node->cond);

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

    value_t Interpreter::visit_Loop(const std::shared_ptr<LoopNode>& node) {
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

    value_t Interpreter::visit_VarDeclaration(const std::shared_ptr<VarDeclarationNode>& node) {
        value_t newValue;
        if (node->initial)
            newValue = visit(node->initial);
        auto type_ = getSymbolFromNode(node->var_type);

        Symbol newVar;
        value_t valueReturn;

        if (node->initial && node->initial->kind() != NodeType::NoOp) {
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

    Interpreting::Symbol* Interpreter::handle_list_type(Interpreting::Symbol* sym, int dimensions) {
        auto prev_sym = sym;
        Interpreting::Symbol* tp;

        if (dimensions == 0) return sym;

        do {
            tp = globalTable.addListType(prev_sym);

            auto element_template_name = "__$" + tp->name + "_list_element";

            auto in_table = currentScope->findSymbol(element_template_name);
            if (!in_table) {
                auto list_el = currentScope->addSymbol({
                    prev_sym,
                    element_template_name
                });

                list_el->is_initialized = true;
            }

            prev_sym = tp;
            dimensions--;
        } while (dimensions > 0);

        return tp;
    }

    value_t Interpreter::visit_ListDeclaration(const std::shared_ptr<ListDeclarationNode>& node) {
        // TODO: Handle the list type.
        auto base_type = getSymbolFromNode(node->var_type);
        Symbol* newVar;
        value_t valueReturn = null;

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

    value_t Interpreter::visit_Assignment(const std::shared_ptr<AssignmentNode>& node) {
        auto varSym = getSymbolFromNode(node->expr);
        auto newValue = visit(node->val);

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
        return null;
    }

    value_t Interpreter::visit_Variable(const std::shared_ptr<VariableNode>& node) {
        auto found = currentScope->findSymbol(node->token.value);

        if (found->value) {
            return (found->value == nullptr) ? null : found->value;
        } else {
            return null;
        }
    }

    value_t Interpreter::visit_Index(const std::shared_ptr<IndexNode>& node) {
        auto visited_val = visit(node->val);

        if (visited_val->type->name == STRING_TP) {
            auto str = Value::as<NormalValue>(visited_val)->as_string();

            auto visited_indx = visit(node->expr);
            auto int_indx = Value::as<NormalValue>(visited_indx)->as_int();

            if (int_indx >= 0 && static_cast<size_t>(int_indx) < str.size()) {
                std::string result(1, str[int_indx]);
                return create_literal_from_string(result, STRING_TP);
            } else if (int_indx < 0 && static_cast<size_t>(abs(int_indx)) <= str.size()) {
                size_t actual_indx = str.size() + int_indx;
                std::string result(1, str[actual_indx]);
                return create_literal_from_string(result, STRING_TP);
            } else {
                throw Exceptions::ValueException(
                        INDX_LST_OB_EXCP,
                        current_line,
                        current_col
                );
            }
        } else {
            auto list_value = Value::as<ListValue>(visited_val)->as_list_value();
            auto visited_indx = visit(node->expr);
            auto int_indx = Value::as<NormalValue>(visited_indx)->as_int();

            if (int_indx >= 0 && static_cast<size_t>(int_indx) < list_value.size()) {
                return list_value[int_indx];
            } else if (int_indx < 0 && static_cast<size_t>(abs(int_indx)) <= list_value.size()) {
                size_t actual_indx = list_value.size() + int_indx;
                return list_value[actual_indx];
            } if (int_indx >= 0 && static_cast<size_t>(int_indx) < list_value.size()) {
                return list_value[int_indx];
            } else if (int_indx < 0 && static_cast<size_t>(abs(int_indx)) <= list_value.size()) {
                size_t actual_indx = list_value.size() - int_indx;
                return list_value[actual_indx];
            } else {
                throw Exceptions::ValueException(
                    INDX_LST_OB_EXCP,
                    current_line,
                    current_col
                );
            }
        }

        return null;
    }

    value_t Interpreter::visit_ListExpression(const std::shared_ptr<ListExpressionNode>& node) {
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

            value_t actual_value;

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

    value_t Interpreter::visit_BinOp(const std::shared_ptr<BinOpNode>& node) {
        // TODO: Add shortcut-circuit evaluation now that I don't need
        //       to check the exact types of the values at runtime.

        auto& opType = node->token.tp;

        if (opType == Lexing::AND ||
            opType == Lexing::OR) {
            return visit_BinOp_bool(node);

        } else if (opType == Lexing::EQU ||
                   opType == Lexing::NEQ) {
            return visit_BinOp_equa(node);
        } else if (opType == Lexing::LT ||
                   opType == Lexing::GT ||
                   opType == Lexing::LET ||
                   opType == Lexing::GET) {
            return visit_BinOp_rela(node);
        } else {
            return visit_BinOp_arit(node);
        }
    }

    value_t Interpreter::visit_BinOp_arit(const std::shared_ptr<BinOpNode>& node) {
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
                    value_t new_list;

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

                        new_list = ListValue::create(
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
                }
                break;
            }
            case Lexing::DIV: {
                if (leftVisited->type == rightVisited->type && leftVisited->type->name == DOUBLE_TP) {
                    auto result = Value::as<NormalValue>(leftVisited)->as_double() /
                                  Value::as<NormalValue>(rightVisited)->as_double();
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
                    auto result = Value::as<NormalValue>(leftVisited)->as_int() %
                                  Value::as<NormalValue>(rightVisited)->as_int();
                    return create_literal(result);
                } else {
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
                        return create_literal((double) result);
                    } else if (leftVisited->type->name == DOUBLE_TP) {
                        auto result = powl(left_as_normal->as_double(), right_as_normal->as_double());
                        return create_literal((double) result);
                    }
                }
                break;
            default:
                return null;
        }
        return null;
    }

    value_t Interpreter::visit_BinOp_equa(const std::shared_ptr<BinOpNode>& node) {
        auto leftVisited = visit(node->left);
        leftVisited->important = true;
        auto rightVisited = visit(node->right);
        leftVisited->important = false;

        switch (node->token.tp) {
            case Lexing::EQU: {
                if (leftVisited == rightVisited)
                    return create_literal(true);

                if (leftVisited->type == null->type || rightVisited->type == null->type) {
                    return create_literal(false);
                }

                if (leftVisited->type->name == INT_TP) {
                    auto coerced = coerce_type(leftVisited, rightVisited);
                    auto left_as_normal = Value::as<NormalValue>(coerced.first);
                    auto right_as_normal = Value::as<NormalValue>(coerced.second);
                    auto result = left_as_normal->as_int() == right_as_normal->as_int();
                    return create_literal(result);
                } else if (leftVisited->type->name == DOUBLE_TP) {
                    auto coerced = coerce_type(leftVisited, rightVisited);
                    auto left_as_normal = Value::as<NormalValue>(coerced.first);
                    auto right_as_normal = Value::as<NormalValue>(coerced.second);
                    auto result = left_as_normal->as_double() == right_as_normal->as_double();
                    return create_literal(result);
                }

                auto left_as_normal = Value::as<NormalValue>(leftVisited);
                auto right_as_normal = Value::as<NormalValue>(rightVisited);

                if (leftVisited->type->name == BOOL_TP) {
                    auto result = left_as_normal->as_bool() == right_as_normal->as_bool();
                    return create_literal(result);
                } else if (leftVisited->type->name == STRING_TP) {
                    auto result = left_as_normal->as_string() == right_as_normal->as_string();
                    return create_literal(result);
                } else {
                    return create_literal(false);
                }
                break;
            }
            case Lexing::NEQ: {
                if (leftVisited == rightVisited)
                    return create_literal(false);

                if (leftVisited->type == null->type || rightVisited->type == null->type) {
                    return create_literal(true);
                }

                if (leftVisited->type->name == INT_TP) {
                    auto coerced = coerce_type(leftVisited, rightVisited);
                    auto left_as_normal = Value::as<NormalValue>(coerced.first);
                    auto right_as_normal = Value::as<NormalValue>(coerced.second);
                    auto result = left_as_normal->as_int() != right_as_normal->as_int();
                    return create_literal(result);
                } else if (leftVisited->type->name == DOUBLE_TP) {
                    auto coerced = coerce_type(leftVisited, rightVisited);
                    auto left_as_normal = Value::as<NormalValue>(coerced.first);
                    auto right_as_normal = Value::as<NormalValue>(coerced.second);
                    auto result = left_as_normal->as_double() != right_as_normal->as_double();
                    return create_literal(result);
                }

                auto left_as_normal = Value::as<NormalValue>(leftVisited);
                auto right_as_normal = Value::as<NormalValue>(rightVisited);

                if (leftVisited->type->name == BOOL_TP) {
                    auto result = left_as_normal->as_bool() != right_as_normal->as_bool();
                    return create_literal(result);
                } else if (leftVisited->type->name == STRING_TP) {
                    auto result = left_as_normal->as_string() != right_as_normal->as_string();
                    return create_literal(result);
                } else {
                    return create_literal(true);
                }
                break;
            }
            default:
                return null;
        }
        return null;
    }

    value_t Interpreter::visit_BinOp_rela(const std::shared_ptr<BinOpNode>& node) {
        auto leftVisited = visit(node->left);
        leftVisited->important = true;
        auto rightVisited = visit(node->right);
        leftVisited->important = false;

        auto coerced = coerce_type(leftVisited, rightVisited);

        auto left_as_normal = Value::as<NormalValue>(coerced.first);
        auto right_as_normal = Value::as<NormalValue>(coerced.second);

        double left_actual_value = 0;
        double right_actual_value = 0;
        if (leftVisited->type->name == INT_TP) {
            left_actual_value = left_as_normal->as_int();
            right_actual_value = right_as_normal->as_int();
        } else {
            left_actual_value = left_as_normal->as_double();
            right_actual_value = right_as_normal->as_double();
        }

        switch (node->token.tp) {
            case Lexing::LT: {
                return create_literal(left_actual_value < right_actual_value);
            }
            case Lexing::GT: {
                return create_literal(left_actual_value > right_actual_value);
            }
            case Lexing::LET:
            {
                return create_literal(left_actual_value <= right_actual_value);
            }
            case Lexing::GET:
            {
                return create_literal(left_actual_value >= right_actual_value);
            }
            default:
                return null;
        }
        return null;
    }

    value_t Interpreter::visit_BinOp_bool(const std::shared_ptr<BinOpNode>& node) {
        auto lhs = Value::as<NormalValue>(visit(node->left))->as_bool();
        switch (node->token.tp) {
            case Lexing::AND:
            {
                auto result = lhs && Value::as<NormalValue>(visit(node->right))->as_bool();
                return create_literal(result);
            }
            case Lexing::OR:
            {
                auto result = lhs || Value::as<NormalValue>(visit(node->right))->as_bool();
                return create_literal(result);
            }
            default:
                return null;
        }
        return null;
    }

    value_t Interpreter::visit_UnaryOp(const std::shared_ptr<UnaryOpNode>& node) {
        auto result = visit(node->ast);

        auto result_as_normal = Value::as<NormalValue>(result);

        switch (node->token.tp) {
            case Lexing::PLUS:
                return result_as_normal;
            case Lexing::MINUS:
                if (result->type->name == INT_TP) {
                    auto actual_value = result_as_normal->as_int();

                    result_as_normal->val = actual_value * -1;
                    return result;
                } else {
                    auto actual_value = result_as_normal->as_double();

                    result_as_normal->val = actual_value * -1;
                    return result;
                }
            default:
                break;
        }

        return null;
    }

    value_t Interpreter::visit_Module(const std::shared_ptr<ModuleNode>& node) {
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

        currentScope->addSymbol({nullptr, node->name.value, moduleValue, false, SymbolType::ModuleSymbol});

        return moduleValue;
    }

    value_t Interpreter::visit_Import(const std::shared_ptr<ImportNode>& node) {
        interpret_as_module(node->path.value, node->name);
        return null;
    }

    value_t Interpreter::visit_Define(const std::shared_ptr<DefineNode>& node) {
        std::vector<std::pair<Symbol*, bool>> as_function_types;
        as_function_types.reserve(node->args.size());

        for(const auto& sm : node->args) {
            // No need to worry if it exists. The SemAn does that.
            auto found_symbol = currentScope->findSymbol(sm.first.value);

            as_function_types.push_back({found_symbol, sm.second});
        }

        Interpreting::Symbol* ret_type_symbol{nullptr};
        if (node->retType.tp != Lexing::NOTHING) {
            ret_type_symbol = currentScope->findSymbol(node->retType.value);
        }

        auto func_name = Symbol::constructFuncTypeName(ret_type_symbol, as_function_types);
        auto func_type = globalTable.addFuncType(ret_type_symbol, func_name);

        currentScope->addAlias(node->name.value, func_type);

        return null;
    }

    value_t Interpreter::visit_FuncExpression(const std::shared_ptr<FuncExpressionNode>& node){
        auto returnType =
                node->retType->kind() == Parsing::NodeType::NoOp
                ? nullptr
                : getSymbolFromNode(node->retType);

        auto paramTypes = getParamTypes(node->params);

        auto typeName = Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(returnType, paramTypes);
        }

        auto funcValue = FunctionValue::create(typeOfFunc, node->params, node->body, currentScope);

        return funcValue;
    }

    value_t Interpreter::visit_FuncDecl(const std::shared_ptr<FuncDeclNode>& node){
        Interpreting::Symbol* returnType = nullptr;

        if (node->retType->kind() != Parsing::NodeType::NoOp) {
            if (node->retType->kind() == Parsing::NodeType::Index) {
                auto current = Node::as<IndexNode>(node->retType)->val;
                int dimensions = 1;

                while (current && current->kind() == Parsing::NodeType::Index) {
                    dimensions++;
                    current = Node::as<IndexNode>(current)->val;
                }

                returnType = handle_list_type(getSymbolFromNode(current), dimensions);
            } else {
                returnType = getSymbolFromNode(node->retType);
            }
        }

        auto paramTypes = getParamTypes(node->params);

        auto typeName = Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(returnType, typeName);
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

    value_t Interpreter::visit_FuncCall(const std::shared_ptr<FuncCallNode>& node) {
        if (call_stack.size() >= MAX_CALL_DEPTH) {
            throw Exceptions::RecursionException(CALL_DEPTH_EXC_EXCP, current_line, current_col);
        }
        if (returning_native) returning_native = nullptr;

        if (node->fname.tp != Lexing::NOTHING){
            auto found_in_natives = native_functions.find(node->fname.value);

            if (found_in_natives != native_functions.end()) {
                auto num_args = node->args.size();
                std::vector<value_t> arguments_visited;

                std::vector<bool> was_important{};
                for(size_t i = 0; i < num_args; i++){
                    auto& arg = node->args[i];
                    auto v = visit(arg);
                    was_important.push_back(v->important);
                    v->important = true;
                    arguments_visited.push_back(std::move(v));
                }

                auto result = found_in_natives->second(arguments_visited);
                for (size_t i = 0; i < arguments_visited.size(); i++) {
                    if (!was_important[i])
                        arguments_visited[i]->important = false;
                }

                return result;
            }
        }

        auto fVal = visit(node->expr);
        if (fVal->kind() == ValueType::NativeFunctionVal) {
            auto as_native = Value::as<NativeFunctionValue>(fVal);
            if (as_native->function_kind == NativeFunctionValue::NativeFunctionType::Simple) {
                std::vector<std::any> args;
                auto &function_params = as_native->arguments;
                for (size_t i = 0; i < node->args.size(); i++) {
                    auto arg = node->args[i];
                    auto val = visit(arg);
                    // Really messy. I dont like this.
                    // I'll worry about making it functional right now.
                    // Efficient and good code later.
                    if (function_params.size() > i) {
                        auto param_type = function_params[i].first;
                        if (val->type != param_type && param_type->is_numeric()) {
                            if (param_type->name == INT_TP) {
                                auto as_double = Value::as<NormalValue>(val)->as_double();
                                val = create_literal((int) as_double);
                            } else {
                                auto as_int = Value::as<NormalValue>(val)->as_int();
                                val = create_literal((double) as_int);
                            }
                        }
                    }
                    args.push_back(Value::as<NormalValue>(val)->val);
                }
                auto result = as_native->fn(args);

                // If the function has return type, return something.
                if (as_native->type->tp) {
                    return NormalValue::create(as_native->type->tp, result);
                }

            } else if (as_native->function_kind == NativeFunctionValue::NativeFunctionType::Values) {
                std::vector<value_t> arguments;
                for (const auto& arg : node->args) {
                    arguments.push_back(visit(arg));
                }
                return as_native->values_fn(arguments);
            }
            return null;
        } else {
            auto as_function_value = Value::as<FunctionValue>(fVal);

            auto funcScope = SymbolTable("func-scope", {}, as_function_value->parentScope);
            auto calleeScope = currentScope;

            std::vector<std::shared_ptr<Node>> newDecls;
            std::vector< std::pair<Lexing::Token, value_t> > initValues;

            for (size_t i = 0; i < as_function_value->params.size(); i++) {
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

            for (size_t i = 0; i < newDecls.size(); i++) {
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
    }

    value_t Interpreter::visit_FuncBody(const std::shared_ptr<FuncBodyNode>& node) {
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

    value_t Interpreter::visit_Return(const std::shared_ptr<ReturnNode>& node) {
        returning = visit(node->val);
        returning->important = true;
        return null;
    }

    value_t Interpreter::visit_Enum(const std::shared_ptr<EnumNode>& node) {
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

    value_t Interpreter::visit_Class(const std::shared_ptr<ClassNode>& node) {
        Symbol* typeSym = nullptr;

        if (node->ty->kind() != Parsing::NodeType::NoOp) {
            typeSym = getSymbolFromNode(node->ty);
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

    value_t Interpreter::visit_ClassBody(const std::shared_ptr<ClassBodyNode>& node) {
        for (auto& st : node->statements) {
            if (st->kind() == NodeType::StaticStatement)
                visit(Node::as<StaticStatementNode>(st)->statement);
        }

        return null;
    }

    value_t Interpreter::visit_ConstructorDecl(const std::shared_ptr<ConstructorDeclNode>& node) {
        Symbol* retType = nullptr;

        auto paramTypes = getParamTypes(node->params);

        auto typeName = Symbol::constructFuncTypeName(retType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(retType, typeName);
        }

        auto funcValue = FunctionValue::create(typeOfFunc, node->params, node->body, currentScope);

        currentScope->addSymbol({typeOfFunc, "constructor", funcValue});

        return null;
    }

    value_t Interpreter::visit_ConstructorCall(const std::shared_ptr<ConstructorCallNode>& node) {
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

        auto as_function_value = Value::as<FunctionValue>(fVal);
        SymbolTable funcScope = {"constructor-scope", {}, as_function_value->parentScope};
        auto calleeScope = currentScope;

        std::vector<std::shared_ptr<Node>> newDecls;
        std::vector< std::pair<Lexing::Token, value_t> > initValues;

        for (size_t i = 0; i < as_function_value->params.size(); i++) {
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

        for (size_t i = 0; i < newDecls.size(); i++) {
            visit(newDecls[i]);

            if (i < initValues.size()) {
                auto newVar = currentScope->findSymbol(initValues[i].first.value);
                newVar->value = initValues[i].second;
            }
        }

        visit(as_function_value->body);
        currentScope = calleeScope;
        call_stack.pop_back();

        constructorParams.clear();
        constructorParams.shrink_to_fit();

        return null;
    }

    value_t Interpreter::visit_ClassInitializer(const std::shared_ptr<ClassInitializerNode>& node) {
        auto classInit = getSymbolFromNode(node->cls);
        auto classVal = Value::as<ClassValue>(classInit->value);

        SymbolTable instanceScope{"instance-" + classInit->name + "-scope", {}, classVal->parentScope};

        auto newInstance = InstanceValue::create(classInit, classVal, instanceScope);
        newInstance->important = true;

        std::vector<value_t> newParams;
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
        auto myInstanceBody = InstanceBodyNode::create(node_as_class_body->statements);

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
            auto inherBody = InstanceBodyNode::create(Node::as<ClassBodyNode>(currentClass->body)->statements);

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
        auto initFuncCall = ConstructorCallNode::create(initID);

        visit(initFuncCall);

        currentScope = tempScope;

        newInstance->important = false;

        return newInstance;
    }

    value_t Interpreter::visit_InstanceBody(const std::shared_ptr<InstanceBodyNode>& node){
        for (auto& st : node->statements) {
            if (st->kind() != NodeType::StaticStatement)
                visit(st);
        }

        return null;
    }

    value_t Interpreter::visit_MemberVar(const std::shared_ptr<MemberVarNode>& node) {
        auto instance = visit(node->inst);
        auto as_instance_value = Value::as<InstanceValue>(instance);

        if (instance == get_null()) {
            throw Exceptions::ValueException("Trying to access a member variable o null instance.");
        }

        auto foundSymbol = as_instance_value->ownScope.findSymbol(node->name.value);

        return foundSymbol->value ? foundSymbol->value : null;
    }

    value_t Interpreter::visit_StaticVar(const std::shared_ptr<StaticVarNode>& node) {
        auto symbol = getSymbolFromNode(node);
        return symbol->value ? symbol->value : null;
    }

    Symbol* Interpreter::getSymbolFromNode(const std::shared_ptr<Node>& mem) {
        Symbol* varSym = nullptr;

        switch (mem->kind()) {
            case NodeType::Variable:
                varSym = currentScope->findSymbol(Node::as<VariableNode>(mem)->token.value);
                break;
            case NodeType::MemberVar:
            {
                auto as_member_node = Node::as<MemberVarNode>(mem);
                auto leftHandSym = getSymbolFromNode(as_member_node->inst);

                if (leftHandSym && leftHandSym->value) {
                    auto theValue = leftHandSym->value;
                    auto as_instance_value = Value::as<InstanceValue>(theValue);

                    varSym = as_instance_value->ownScope.findSymbol(as_member_node->name.value);
                }
                break;
            }
            case NodeType::StaticVar:
            {
                auto as_static_var = Node::as<StaticVarNode>(mem);
                auto leftHandSym = getSymbolFromNode(as_static_var->inst);

                auto theValue = leftHandSym->value;
                if (theValue->kind() == ValueType::ModuleVal) {
                    varSym = Value::as<ModuleValue>(theValue)->ownScope.findSymbol(as_static_var->name.value, false);
                } else if (theValue->kind() == ValueType::ClassVal) {
                    varSym = Value::as<ClassValue>(theValue)->getStaticVarSymbol(as_static_var->name.value);
                } else if (theValue->kind() == ValueType::InstanceVal) {
                    varSym = Value::as<InstanceValue>(theValue)->getStaticVarSymbol(as_static_var->name.value);
                } else if (theValue->kind() == ValueType::EnumVal) {
                    auto as_enum_value = Value::as<EnumValue>(theValue);
                    auto sm = as_enum_value->ownScope.findSymbol(as_static_var->name.value, false);
                    return sm;
                }

                break;
            }
            case NodeType::Index:
            {
                auto as_index_node = Node::as<IndexNode>(mem);
                auto visited_source = visit(as_index_node->val);
                auto visited_indx = visit(as_index_node->expr);
                auto& as_list = Value::as<ListValue>(visited_source)->elements;
                auto as_int = Value::as<NormalValue>(visited_indx)->as_int();

                auto as_size_t = static_cast<size_t>(as_int);
                // TODO: Add funcionality of reverse indexing.
                if (as_int < 0) {
                    if ((as_list.size() + as_size_t) < 0) {
                        throw Exceptions::ValueException(
                                INDX_STR_OB_EXCP,
                                current_line,
                                current_col
                        );
                    }

                    as_int = as_list.size() + as_size_t;
                } else {
                    if (as_size_t > as_list.size()-1) {
                        throw Exceptions::ValueException(
                                INDX_STR_OB_EXCP,
                                current_line,
                                current_col
                        );
                    }
                }
                return &as_list[as_int];
            }
            default:
                break;
        }

        return varSym;
    }

    void Interpreter::interpret(std::string code) {
        parser.set_text(std::move(code));

        auto root = parser.program();

        analyzer->visit(root);

        call_stack.push_back({"global", 1, 1});
        visit(root);
        call_stack.pop_back();
    }

    value_t Interpreter::eval(std::string code) {

        call_stack.push_back({"global", 1, 1});
        parser.set_text(std::move(code));

        auto statements = parser.program_content();

    //    try{
        currentScope = &replScope;

        auto result = null;
        for (const auto& node : statements) {
            analyzer->from_repl(node);
            result = visit(node);
        }

        currentScope = &globalTable;
    //    }
        call_stack.pop_back();

        return result;
    }

    void Interpreter::set_repl_last(value_t v) {
        auto& last_value_symbol = replScope.symbols.at("_");
        // Note: This may have some mistakes if the value has no references.
        // But by the nature of the repl, there shouldn't be much problems
        // _ Should be treated as any value that was just created on the spot.
        last_value_symbol.value = std::move(v);
    }

    value_t Interpreter::interpret_as_module(const std::string &path, const Lexing::Token& name) {
        bool has_extension = ends_with(path, ".odo");

        std::string full_path = path;
        if (!has_extension)
            full_path += ".odo";
        auto filename = io::get_file_name(full_path, true);
        if (name.tp != Lexing::NOTHING)
            filename = name.value;

        std::string code = io::read_file(full_path);
        Parsing::Parser pr;
        pr.set_text(code);

        auto body = pr.program_content();

        auto file_module = ModuleNode::create(
            Lexing::Token(Lexing::STR, filename),
            body
        );

        call_stack.push_back({"module '" + full_path + "'", 1, 1});

        auto result = visit(file_module);

        call_stack.pop_back();
        return result;
    }

    std::vector<std::pair<Symbol*, bool>> Interpreter::getParamTypes(const std::vector<std::shared_ptr<Node>>& params) {
        std::vector<std::pair<Symbol*, bool>> ts;

        for (const auto& par : params) {
            switch (par->kind()) {
                case NodeType::VarDeclaration: {
                    auto as_var_declaration_node = Node::as<VarDeclarationNode>(par);
                    auto ft = getSymbolFromNode(as_var_declaration_node->var_type);
                    auto is_not_optional = as_var_declaration_node->initial && as_var_declaration_node->initial->kind() != NodeType::NoOp;
                    ts.emplace_back(ft, is_not_optional);
                    break;
                }
                case NodeType::ListDeclaration: {// FIXME: List types are registered as their basetype and not as listtype
                    auto as_var_declaration_node = Node::as<ListDeclarationNode>(par);
                    auto ft = getSymbolFromNode(as_var_declaration_node->var_type);
                    auto is_not_optional = as_var_declaration_node->initial && as_var_declaration_node->initial->kind() != NodeType::NoOp;
                    ts.emplace_back(ft, is_not_optional);

                    break;
                }
                default:
                    break;
            }
        }

        return ts;
    }
}