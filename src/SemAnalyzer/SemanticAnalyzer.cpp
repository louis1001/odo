//
// Created by Luis Gonzalez on 12/19/20.
//

#include "SemAnalyzer/SemanticAnalyzer.h"

#include <utility>
#include <IO/io.h>
#include "Exceptions/exception.h"
#include "Interpreter/Interpreter.h"

#define TEST_SEMANTICS

namespace Odo::Semantics {
    using namespace Parsing;
    // This should initialize the symbol tables and basic types, like the Interpreter.
    SemanticAnalyzer::SemanticAnalyzer(Interpreting::Interpreter& inter_): inter(inter_) {
        currentScope = &inter.globalTable;
        globalScope = {"global-semantics-context", {}, &inter.globalTable};

        type_int = &inter.globalTable.symbols[INT_TP];
        type_double = &inter.globalTable.symbols[DOUBLE_TP];
        type_string = &inter.globalTable.symbols[STRING_TP];
        type_bool = &inter.globalTable.symbols[BOOL_TP];

        replScope = { "repl-analyzer", {}, &globalScope };

        native_function_data[WRITE_FN] = {};
        native_function_data[WRITELN_FN] = {};
        native_function_data[MOVE_CRSR_FN] = {};
        native_function_data[FACTR_FN] = {type_int, true, false};
        native_function_data[LENGTH_FN] = {type_int};
        native_function_data[FROM_ASCII_FN] = {type_string};
        native_function_data[TO_ASCII_FN] = {type_int};
        native_function_data[POW_FN] = {type_double};
        native_function_data[SQRT_FN] = {type_double};
        native_function_data[SIN_FN] = {type_double};
        native_function_data[COS_FN] = {type_double};
        native_function_data[FLOOR_FN] = {type_int};
        native_function_data[TRUNC_FN] = {type_int};
        native_function_data[ROUND_FN] = {type_double};
        native_function_data[READ_FN] = {type_string};
        native_function_data[READ_INT_FN] = {type_int};
        native_function_data[READ_DOUBLE_FN] = {type_double};
        native_function_data[RAND_FN] = {type_double};
        native_function_data[RAND_INT_FN] = {type_int};
        native_function_data[POP_FN] = {inter.any_type()};
        native_function_data[PUSH_FN] = {};
        native_function_data[TYPEOF_FN] = {type_string};

        native_function_data[TO_ABS_PATH_FN] = {type_string};
        native_function_data[GET_CWD_FN] = {type_string};
        native_function_data[READ_FILE_FN] = {type_string};
        native_function_data[PATH_EXISTS_FN] = {type_bool};
        native_function_data[IS_DIR_FN] = {type_bool};
        native_function_data[IS_FILE_FN] = {type_bool};

        auto lst_type = handle_list_type(type_string);
        native_function_data[LIST_DIR_FN] = {lst_type};
        native_function_data[CREATE_FILE_FN] = {};
        native_function_data[WRITE_TO_FILE_FN] = {};
        native_function_data[APPEND_TO_FILE_FN] = {};
        push_lazy_scope();
    }

    SemanticAnalyzer::~SemanticAnalyzer() {
        for (const auto& instance_context_entry : instance_contexts) {
            auto table = instance_context_entry.second;
            // Skip the fist cause it was not created with new.
            for (size_t i = 1; i < table.size(); i++) {
                auto the_table = table[i];
                delete the_table;
            }
        }
        deactivate_cleanup = true;
    }

    Interpreting::SymbolTable* SemanticAnalyzer::add_semantic_context(Interpreting::Symbol* sym, Interpreting::SymbolTable tb) {
        semantic_contexts.emplace(sym, std::move(tb));

        sym->ondestruction = [this](auto* sym){
            if (deactivate_cleanup) return;
            semantic_contexts.erase(sym);
        };

        return &semantic_contexts.find(sym)->second;
    }

    void SemanticAnalyzer::add_lazy_check(Interpreting::Symbol* sym, lazy_check check) {
        sym->has_been_checked = false;
        current_lazy_scope->insert({sym, std::move(check)});
    }

    void SemanticAnalyzer::consume_lazy(Interpreting::Symbol* sym) {
        if (!lazy_scope_stack.empty()) {
            // Travel backwards through the
            for(auto cur = lazy_scope_stack.rbegin(); cur != lazy_scope_stack.rend(); ++cur) {
                auto in_scope = cur->find(sym);
                if (in_scope != cur->end()) {
                    sym->has_been_checked = true;
                    auto checks = in_scope->second;
                    cur->erase(in_scope);
                    try {
                        checks.body(checks.parent);
                    } catch (Exceptions::OdoException& e) {
                        if (checks.on_error) {
                            checks.on_error();
                        }
                        if (checks.parent) {
                            checks.parent->removeSymbol(sym);
                        }
                        throw e;
                    }
                    return;
                }
            }
        }
    }

    void SemanticAnalyzer::push_lazy_scope() {
        lazy_scope_stack.emplace_back();
        current_lazy_scope = &lazy_scope_stack.back();
    }

    void SemanticAnalyzer::pop_lazy_scope() {
        if (!lazy_scope_stack.empty() && current_lazy_scope) {
            // I am doing a copy of the scope, so if there's any change inside of it while iterating
            // This copy will have no knowledge of it.
            auto lz_scope = *current_lazy_scope;
            for (const auto& symbol_analyzer : lz_scope) {
                // Copy. Unnecessary? Probably not if there's an error.
                auto sym = symbol_analyzer.first;
                if (!sym->has_been_checked){
                    auto checks = symbol_analyzer.second;
                    // Do it before so that recursive calls don't consider themselves not checked
                    symbol_analyzer.first->has_been_checked = true;
                    try {
                        checks.body(checks.parent);
                    } catch (Exceptions::OdoException& e) {
                        if (checks.on_error) {
                            checks.on_error();
                        }
                        if (checks.parent) {
                            checks.parent->removeSymbol(sym);
                        }
                        throw e;
                    }
                }
            }
            lazy_scope_stack.pop_back();
            current_lazy_scope = lazy_scope_stack.empty() ? nullptr : &lazy_scope_stack.back();
        }
    }

    Interpreting::SymbolTable* SemanticAnalyzer::add_semantic_context(Interpreting::Symbol* sym, std::string name) {
        return add_semantic_context(sym, {std::move(name), {}, currentScope});
    }

    Interpreting::SymbolTable* SemanticAnalyzer::get_semantic_context(Interpreting::Symbol* sym) {
        auto in_my_map = semantic_contexts.find(sym);
        if (in_my_map != semantic_contexts.end()) return &in_my_map->second;

        return nullptr;
    }

    Interpreting::SymbolTable *
    SemanticAnalyzer::add_function_semantic_context(Interpreting::Symbol* sym, std::string name, SemanticAnalyzer::arg_types tps) {
        auto func_table = add_semantic_context(sym, std::move(name));

        functions_context.insert(std::pair(sym, std::move(tps)));

        sym->ondestruction = nullptr;
        return func_table;
    }

    SemanticAnalyzer::arg_types SemanticAnalyzer::get_function_semantic_context(Interpreting::Symbol* sym) {
        auto in_my_map = functions_context.find(sym);
        if (in_my_map != functions_context.end()) return in_my_map->second;

        return {};
    }

    bool SemanticAnalyzer::counts_as(Interpreting::Symbol* type1, Interpreting::Symbol* type2) {
        if (type2 == inter.any_type()) return true;
        if (type1 == inter.null->type && type2->kind == Interpreting::SymbolType::ClassType) return true;

        if (type1->is_numeric() && type2->is_numeric()) return true;

        if (type1->kind == type2->kind && type1->kind == Interpreting::SymbolType::ListType) {
            return counts_as(type1->tp, type2->tp);
        }

        auto curr = type1;
        while (curr) {
            if (curr == type2) return true;

            curr = curr->tp;
        }

        return false;
    }

    Interpreting::Symbol* SemanticAnalyzer::string_to_list_type(const std::string& name) {
        std::string_view shortened_name = name;
        int dimensions = 0;

        while (shortened_name.ends_with("[]")) {
            shortened_name.remove_suffix(2);
            dimensions++;
        }

        auto base_type = currentScope->findSymbol(static_cast<std::string>(shortened_name));

        if (dimensions == 0) return base_type;

        return handle_list_type(base_type, dimensions);
    }

    Interpreting::Symbol* SemanticAnalyzer::handle_list_type(Interpreting::Symbol* sym, int dimensions) {
        auto prev_sym = sym;
        Interpreting::Symbol* tp;

        if (dimensions == 0) return sym;

        do {
            tp = inter.globalTable.addListType(prev_sym);
    
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

    Interpreting::Symbol* SemanticAnalyzer::getStaticFromClass(Interpreting::Symbol* cls, const std::shared_ptr<Parsing::StaticVarNode>& var) {
        auto current_class = cls;
        do {
            auto class_context = get_semantic_context(current_class);
            auto in_class = class_context->findSymbol(var->name.value, false);
            if (in_class) {
                // FixMe: optimize this without having scoping errors.
//                var->inst = VariableNode::create({Lexing::TokenType::ID, current_class->name});
                return in_class;
            }
            current_class = current_class->tp;
        } while (current_class);

        return nullptr;
    }

    Interpreting::Symbol* SemanticAnalyzer::getSymbolFromNode(const std::shared_ptr<Parsing::Node>& mem) {
        Interpreting::Symbol* varSym = nullptr;

        // TODO: Fix this to not return early
        //       Just assign to varSym.
        switch (mem->kind()) {
            case NodeType::Variable:
                varSym = currentScope->findSymbol(Node::as<VariableNode>(mem)->token.value);
                break;
            case NodeType::MemberVar:
            {
                auto as_member_node = Node::as<MemberVarNode>(mem);
                auto leftHandSym = getSymbolFromNode(as_member_node->inst);

                if (leftHandSym) {
                    if (leftHandSym->tp->kind != Interpreting::SymbolType::ClassType) {
                        throw Exceptions::ValueException(
                                "'" + leftHandSym->name + NOT_VALID_INST_EXCP,
                                mem->line_number,
                                mem->column_number
                        );
                    }

                    auto template_name = "__$" + leftHandSym->tp->name + "_instance_template";
                    auto parentScope = leftHandSym->tp->table;
                    auto instance_template = parentScope->findSymbol(template_name);

                    auto instance_scopes = instance_contexts.at(instance_template);

                    for (auto tab : instance_scopes) {
                        auto foundSymbol = tab->findSymbol(as_member_node->name.value, false);
                        if (foundSymbol) {
                            return foundSymbol;
                        }
                    }
                    throw Exceptions::NameException(
                        NO_MEM_CALLED_EXCP + as_member_node->name.value + IN_THE_INST_EXCP,
                        as_member_node->line_number,
                        as_member_node->column_number
                    );
                } else {
                    throw Exceptions::ValueException(
                            SYM_NOT_DEFINED_EXCP,
                            mem->line_number,
                            mem->column_number
                    );
                }
                break;
            }
            case NodeType::StaticVar:
            {
                auto as_static_var = Node::as<StaticVarNode>(mem);
                auto leftHandSym = getSymbolFromNode(as_static_var->inst);

                if (leftHandSym) {
                    if (leftHandSym->kind == Interpreting::SymbolType::ModuleSymbol) {
                        auto module_context = get_semantic_context(leftHandSym);
                        varSym = module_context->findSymbol(as_static_var->name.value, false);
                    } else if (leftHandSym->kind == Interpreting::SymbolType::ClassType) {
                        varSym = getStaticFromClass(leftHandSym, as_static_var);
                    } else if (leftHandSym->kind == Interpreting::SymbolType::EnumType) {
                        auto context = get_semantic_context(leftHandSym);
                        auto sm = context->findSymbol(as_static_var->name.value, false);
                        if (sm) {
                            return sm;
                        } else {
                            throw Exceptions::NameException(
                                    as_static_var->name.value + NOT_VARIANT_IN_ENUM_EXCP,
                                    mem->line_number,
                                    mem->column_number
                            );
                        }
                    } else if (leftHandSym->tp && leftHandSym->tp->kind == Interpreting::SymbolType::ClassType) {
                        varSym = getStaticFromClass(leftHandSym->tp, as_static_var);
                    } else {
                        throw Exceptions::NameException(
                                //TODO Change to cannot read static
                                INVALID_STATIC_OP_EXCP,
                                mem->line_number,
                                mem->column_number
                        );
                    }
                } else {
                    throw Exceptions::NameException(
                            UNKWN_VAL_IN_STATIC_EXCP,
                            mem->line_number,
                            mem->column_number
                    );
                }

                break;
            }
            case NodeType::Index:
            {
                auto as_index_node = Node::as<IndexNode>(mem);
                auto visited_source = visit(as_index_node->val);

                if (!visited_source.type) {
                    throw Exceptions::ValueException(
                            NO_VALUE_TO_INDEX_EXCP,
                            as_index_node->line_number,
                            as_index_node->column_number
                    );
                }

                if (visited_source.type->kind == Interpreting::SymbolType::ListType) {
                    auto visited_indx = visit(as_index_node->expr);

                    if (!visited_indx.type) {
                        throw Exceptions::ValueException(
                                INDEX_MUST_BE_VALID_EXCP,
                                as_index_node->expr->line_number,
                                as_index_node->expr->column_number
                        );
                    }

                    if (visited_indx.type->name == INT_TP) {
                        auto element_template_name = "__$" + visited_source.type->name + "_list_element";

                        varSym = currentScope->findSymbol(element_template_name);
                    } else {
                        throw Exceptions::TypeException(
                                LST_ONLY_INDX_NUM_EXCP,
                                as_index_node->line_number,
                                as_index_node->column_number
                        );
                    }
                } else {
                    throw Exceptions::ValueException(
                            ASS_TO_INVALID_INDX_EXCP,
                            as_index_node->line_number,
                            as_index_node->column_number
                    );
                }
                break;
            }
            default:
                break;
        }

        if (varSym && !varSym->has_been_checked) {
            consume_lazy(varSym);
        }

        return varSym;
    }

    NodeResult SemanticAnalyzer::visit(const std::shared_ptr<Parsing::Node>& node) {
//        current_line = node->line_number;
//        current_col = node->column_number;
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
                 return {};

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
                if (!inside_loop) {
                    throw Exceptions::SemanticException(
                        USE_OF_STA_EXCP BREAK_TK OUTS_A_LOOP_EXCP,
                        node->line_number,
                        node->column_number
                    );
                }
                return {};
            case NodeType::Continue:
                if (!inside_loop) {
                    throw Exceptions::SemanticException(
                            USE_OF_STA_EXCP CONTINUE_TK OUTS_A_LOOP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
                return {};
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
                // return visit_ConstructorCall(Node::as<ConstructorCallNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::InstanceBody:
                 return visit_InstanceBody(Node::as<InstanceBodyNode>(node));
            case NodeType::ClassInitializer:
                 return visit_ClassInitializer(Node::as<ClassInitializerNode>(node));
            case NodeType::StaticStatement:
                 throw Exceptions::SemanticException(
                        STATIC_ONLY_CLASS_EXCP,
                        node->line_number,
                        node->column_number
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
                (void)0;
            case NodeType::Null:
                return {inter.get_null()->type, true, false};
        }

#ifdef TEST_SEMANTICS
        NOT_IMPLEMENTED("Semantics");
#endif
        return {};
    }

    NodeResult SemanticAnalyzer::visit_Double(const std::shared_ptr<Parsing::DoubleNode>& node) {
        return {type_double, true, false};
    }

    NodeResult SemanticAnalyzer::visit_Int(const std::shared_ptr<Parsing::IntNode>& node) {
        return {type_int, true, false};
    }

    NodeResult SemanticAnalyzer::visit_Str(const std::shared_ptr<Parsing::StrNode>& node) {
        return {type_string, true, false};
    }

    NodeResult SemanticAnalyzer::visit_Bool(const std::shared_ptr<Parsing::BoolNode>& node) {
        return {type_bool, true, false};
    }

    NodeResult SemanticAnalyzer::visit_BinOp(const std::shared_ptr<Parsing::BinOpNode>& node) {
        auto leftVisited = visit(node->left);
        auto rightVisited = visit(node->right);

        if (!leftVisited.type) {
            //Error! Left operand in binary operation has no value.
            throw Exceptions::ValueException(
                LEFT_OP_BIN_NO_VAL_EXCP,
                node->line_number,
                node->column_number
            );
        }

        if (!rightVisited.type) {
            //Error! Right operand in binary operation has no value.
            throw Exceptions::ValueException(
                RIGHT_OP_BIN_NO_VAL_EXCP,
                node->line_number,
                node->column_number
            );
        }

        auto both_numerical = leftVisited.type->is_numeric() && rightVisited.type->is_numeric();
        auto same_types = leftVisited.type == rightVisited.type;

        switch (node->token.tp) {
            case Lexing::PLUS: {
                if (leftVisited.type->kind == Interpreting::SymbolType::ListType) {
                    auto element_template_name = "__$" + leftVisited.type->name + "_list_element";
                    auto template_symbol = currentScope->findSymbol(element_template_name);

                    if (rightVisited.type->kind == Interpreting::SymbolType::ListType) {
                        if (!counts_as(rightVisited.type->tp, leftVisited.type->tp)) {
                            // Error! Contenating lists with incompatible types.
                            throw Exceptions::TypeException(
                                CONC_LST_INCOM_TPS_EXCP,
                                node->line_number,
                                node->column_number
                            );
                        }
                    } else {
                        if (!counts_as(rightVisited.type, leftVisited.type->tp)) {
                            // Error! Appending to list a symbol of incompatible type.
                            throw Exceptions::TypeException(
                                APP_LST_INCOM_TP_EXCP,
                                node->line_number,
                                node->column_number
                            );
                        }
                    }
                    return {
                            leftVisited.type,
                            template_symbol->content_is_constant,
                            template_symbol->content_has_side_effects
                    };
                } else if (leftVisited.type->name == STRING_TP || rightVisited.type->name == STRING_TP) {
                    return {type_string};
                } else if (both_numerical) {
                    if (same_types && leftVisited.type->name == INT_TP) {
                        return {type_int};
                    }

                    return {type_double};
                } else {
                    throw Exceptions::TypeException(
                            ADD_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            }
            case Lexing::MINUS:
                if (both_numerical) {
                    if (same_types && leftVisited.type->name == INT_TP) {
                        return {type_int};
                    }

                    return {type_double};
                } else {
                    throw Exceptions::TypeException(
                            SUB_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            case Lexing::MUL: {
                if (both_numerical) {
                    if (same_types && leftVisited.type->name == INT_TP) {
                        return {type_int};
                    }

                    return {type_double};
                } else if (leftVisited.type->kind == Interpreting::SymbolType::ListType) {
                    if (rightVisited.type->name != INT_TP) {
                        // Error! List can only be multiplied with ints.
                        throw Exceptions::TypeException(
                            LST_ONLY_MUL_INT_EXCP,
                            node->line_number,
                            node->column_number
                        );
                    }

                    // TODO: Worry about the flags later.
                    return leftVisited;
                } else if (leftVisited.type->name == STRING_TP) {
                    if (rightVisited.type->name != INT_TP) {
                        // Error! Strings can only be multiplied with ints.
                        throw Exceptions::TypeException(
                            STR_ONLY_MUL_INT_EXCP,
                            node->line_number,
                            node->column_number
                        );
                    }
                    return {type_string};
                } else {
                    throw Exceptions::TypeException(
                            MUL_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            }
            case Lexing::DIV: {
                if (both_numerical) {
                    // Just return double, even if both are int.
                    // TODO: I need to update the interpreter with that.
                    return {type_double};
                } else {
                    throw Exceptions::TypeException(
                            DIV_ONLY_DOB_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            }
            case Lexing::MOD:
                if (same_types && leftVisited.type->name == INT_TP) {
                    return {type_int};
                } else {
                    throw Exceptions::TypeException(
                            MOD_ONLY_INT_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            case Lexing::POW:
                if (both_numerical) {
                    if (same_types && leftVisited.type->name == "int") {
                        return {type_int};
                    }

                    return {type_double};
                } else {
                    throw Exceptions::TypeException(
                            POW_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            case Lexing::EQU:
            case Lexing::NEQ:
                if (same_types || leftVisited.type == inter.null->type || rightVisited.type == inter.null->type) {
                    return {type_bool};
                }

                // Else, fallback
            case Lexing::LT:
            case Lexing::GT:
            case Lexing::LET:
            case Lexing::GET:
                if (both_numerical) {
                    return {type_bool};
                } else {
                    throw Exceptions::TypeException(
                            COM_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            case Lexing::AND:
            case Lexing::OR:
                if (same_types && leftVisited.type->name == BOOL_TP) {
                    return {type_bool};
                }else {
                    throw Exceptions::TypeException(
                            LOG_ONLY_BOOL_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            default:
                break;
        }

        return {};
    }

    NodeResult SemanticAnalyzer::visit_UnaryOp(const std::shared_ptr<Parsing::UnaryOpNode>& node) {
        auto result = visit(node->ast);

        if (result.type) {
            if (result.type->name == INT_TP || result.type->name == DOUBLE_TP) {
                return result;
            }
        }

        throw Exceptions::TypeException(
                UNA_ONLY_NUM_EXCP,
                node->line_number,
                node->column_number
        );
    }

    NodeResult SemanticAnalyzer::visit_TernaryOp(const std::shared_ptr<Parsing::TernaryOpNode>& node) {
        auto val_cond = visit(node->cond);

        // The condition of the ternary operator has to be boolean
        if (!val_cond.type || val_cond.type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    COND_TERN_MUST_BOOL_EXCP,
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        // In the SemanticAnalyzer I dont need to check for the condition, just visit every branch.
        auto true_result = visit(node->trueb);

        auto false_result = visit(node->falseb);

        if (!true_result.type || !false_result.type) {
            // Error! Ternary operator branches must be valid expressions. (Must return value)
            throw Exceptions::ValueException(
                    BRANCHES_MUST_RETURN_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        if (!counts_as(false_result.type, true_result.type)) {
            // Error! Both branches must return the same value
            // TODO: Add type coersion
            throw Exceptions::TypeException(
                BOTH_BRANCH_SAME_TYPE_EXCP,
                node->line_number,
                node->column_number
            );
        }

        return {
            true_result.type,
            true_result.is_constant && false_result.is_constant,
            true_result.has_side_effects || false_result.has_side_effects
        };
    }

    NodeResult SemanticAnalyzer::visit_If(const std::shared_ptr<Parsing::IfNode>& node) {
        auto val_cond = visit(node->cond);

        // The condition of the if statement has to be boolean
        // TODO: Add separate error message for when there is no type.
        //      This is for all things that take a condition.
        if (!val_cond.type || val_cond.type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    COND_IF_MUST_BOOL_EXCP,
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        visit(node->trueb);

        if (node->falseb)
            visit(node->falseb);

        return {};
    }

    NodeResult SemanticAnalyzer::visit_For(const std::shared_ptr<Parsing::ForNode>& node) {
        auto forScope = Interpreting::SymbolTable("for:loop", {}, currentScope);
        currentScope = &forScope;
        visit(node->ini);

        auto val_cond = visit(node->cond);

        // The condition of the if statement has to be boolean
        if (!val_cond.type || val_cond.type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    COND_IF_MUST_BOOL_EXCP,
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        visit(node->incr);

        bool prev_state = inside_loop;
        inside_loop = true;
        visit(node->body);
        inside_loop = prev_state;

        currentScope = forScope.getParent();

        return {};
    }

    NodeResult SemanticAnalyzer::visit_ForEach(const std::shared_ptr<Parsing::ForEachNode>& node) {
        auto forScope = Interpreting::SymbolTable("foreach:loop", {}, currentScope);
        currentScope = &forScope;

        auto lst_value = visit(node->lst);

        if (!lst_value.type) {
            throw Exceptions::ValueException(
                NOTHING_TO_ITERATE_EXCP,
                node->line_number,
                node->column_number
            );
        }

        if (lst_value.type->kind == Interpreting::SymbolType::ListType) {
            std::shared_ptr<Node> iterator_decl;
            auto empty_initial = NoOpNode::create();
            auto element_tp = Lexing::Token(Lexing::TokenType::ID, lst_value.type->tp->name);
            iterator_decl = VarDeclarationNode::create(
                VariableNode::create(element_tp),
                node->var, std::move(empty_initial)
            );

            visit(iterator_decl);
            currentScope->findSymbol(node->var.value)->is_initialized = true;

            visit(node->body);
        } else if (lst_value.type->name == STRING_TP) {
            auto iterator_decl = VarDeclarationNode::create(
                VariableNode::create(Lexing::Token(Lexing::TokenType::ID, STRING_TP)),
                node->var,
                NoOpNode::create()
            );
            visit(iterator_decl);
            currentScope->findSymbol(node->var.value)->is_initialized = true;

            bool prev_state = inside_loop;
            inside_loop = true;
            visit(node->body);
            inside_loop = prev_state;

        } else {
            throw Exceptions::ValueException(
                    FOREACH_ONLY_LIST_STR_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        currentScope = forScope.getParent();
        return {};
    }

    NodeResult SemanticAnalyzer::visit_FoRange(const std::shared_ptr<Parsing::FoRangeNode>& node) {
        auto forScope = Interpreting::SymbolTable("forange:loop", {}, currentScope);
        currentScope = &forScope;

        auto first_visited = visit(node->first);
        //TODO: Add "is_numeric" function to types, not values.
        if (!first_visited.type || !(first_visited.type->name == DOUBLE_TP || first_visited.type->name == INT_TP)) {
            throw Exceptions::ValueException(
                    VAL_RANGE_NUM_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        if (node->second && node->second->kind() != NodeType::NoOp) {
            auto second_visited = visit(node->second);
            if (!second_visited.type || !(second_visited.type->name == DOUBLE_TP || second_visited.type->name == INT_TP)) {
                throw Exceptions::ValueException(
                        VAL_RANGE_NUM_EXCP,
                        node->line_number,
                        node->column_number
                );
            }
        }

        bool use_iterator = node->var.tp != Lexing::NOTHING;

        if (use_iterator) {
            std::shared_ptr<Node> iterator_decl = VarDeclarationNode::create(
                VariableNode::create(Lexing::Token(Lexing::TokenType::ID, INT_TP)),
                node->var,
                NoOpNode::create()
            );
            visit(iterator_decl);
            currentScope->findSymbol(node->var.value)->is_initialized = true;
        }

        bool prev_state = inside_loop;
        inside_loop = true;
        visit(node->body);
        inside_loop = prev_state;

        currentScope = forScope.getParent();
        return {};
    }

    NodeResult SemanticAnalyzer::visit_While(const std::shared_ptr<Parsing::WhileNode>& node) {
        auto whileScope = Interpreting::SymbolTable("while:loop", {}, currentScope);
        currentScope = &whileScope;
        auto val_cond = visit(node->cond);

        // The condition of the while statement has to be boolean
        if (!val_cond.type || val_cond.type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    COND_WHILE_MUST_BOOL_EXCP,
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        bool prev_state = inside_loop;
        inside_loop = true;
        visit(node->body);
        inside_loop = prev_state;

        currentScope = whileScope.getParent();

        return {};
    }

    NodeResult SemanticAnalyzer::visit_Loop(const std::shared_ptr<Parsing::LoopNode>& node) {
        bool prev_state = inside_loop;
        inside_loop = true;
        visit(node->body);
        inside_loop = prev_state;
        return {};
    }


    NodeResult SemanticAnalyzer::visit_Index(const std::shared_ptr<Parsing::IndexNode>& node) {
        auto visited_val = visit(node->val);

        if (!visited_val.type) {
            // Error! Using index operator where there's no value to index.
            throw Exceptions::ValueException(
                NO_VALUE_TO_INDEX_EXCP,
                node->line_number,
                node->column_number
            );
        }

        if (visited_val.type->name == STRING_TP) {
            auto visited_indx = visit(node->expr);
            if (visited_indx.type->name != INT_TP) {
                throw Exceptions::TypeException(
                    STR_ONLY_INDX_NUM_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            return {type_string, visited_val.is_constant, visited_val.has_side_effects};
        } else if (visited_val.type->kind == Interpreting::SymbolType::ListType){
            auto visited_indx = visit(node->expr);
            if (visited_indx.type->name != INT_TP) {
                throw Exceptions::TypeException(
                    LST_ONLY_INDX_NUM_EXCP,
                    node->line_number,
                    node->column_number
                );
            }
            return {visited_val.type->tp, visited_val.is_constant};
        } else {
            throw Exceptions::ValueException(
                    INDX_ONLY_LST_STR_EXCP,
                    node->line_number,
                    node->column_number
            );
        }
    }

    NodeResult SemanticAnalyzer::visit_Block(const std::shared_ptr<Parsing::BlockNode>& node) {
        auto blockScope = Interpreting::SymbolTable("block_scope", {}, currentScope);
        currentScope = &blockScope;
        push_lazy_scope();

        for (const auto& statement : node->statements) {
            visit(statement);
        }

        pop_lazy_scope();

        currentScope = blockScope.getParent();
        return {};
    }

    NodeResult SemanticAnalyzer::visit_VarDeclaration(const std::shared_ptr<Parsing::VarDeclarationNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        auto type_ = getSymbolFromNode(node->var_type);

        if (type_ == nullptr) {
            throw Exceptions::NameException(
                    UNKWN_TYPE_EXCP "?'.",
                    node->line_number,
                    node->column_number
            );
        } else if (!type_->has_been_checked) {
            consume_lazy(type_);
        }

        Interpreting::Symbol newVar = Interpreting::Symbol{
            type_,
            node->name.value
        };
        // TODO: Where should the information for the initialization go?
        // If a variable is constant, should it be replaced by the calculation of it's result?

        if (node->initial && node->initial->kind() != NodeType::NoOp) {
            auto newValue = visit(node->initial);

            if (!newValue.type) {
                throw Exceptions::ValueException(
                    VAR_INIT_MUST_BE_VALID_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            if (!counts_as(newValue.type, type_)) {
                throw Exceptions::TypeException(
                        INVALID_DECL_TYPE_EXCP + type_->name + WITH_VAL_OF_TYPE_EXCP + newValue.type->name,
                        node->line_number,
                        node->column_number
                );
            }

            if (type_->name == ANY_TP) {
                newVar.tp = newValue.type;
                // For the moment. I might need to add  it later just in case
//                node->var_type = Lexing::Token(Lexing::TokenType::ID, newValue.type->name);
            }

            // For the moment, type coercion happens at runtime. Will work on that when I figure out
            // how to change the AST from here.
            newVar.is_initialized = true;
            newVar.content_is_constant = newValue.is_constant;
            newVar.content_has_side_effects = newValue.has_side_effects;
        }

        currentScope->addSymbol(newVar);
        return {};
    }

    NodeResult SemanticAnalyzer::visit_ListDeclaration(const std::shared_ptr<Parsing::ListDeclarationNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        auto base_type = getSymbolFromNode(node->var_type);
        if (!(base_type && base_type->isType)) {
            throw Exceptions::TypeException(
                    INVALID_TYPE_EXCP "?'.",
                    node->line_number,
                    node->column_number
            );
        }

        Interpreting::Symbol* list_type = handle_list_type(base_type, node->dim);
        bool was_init = false;
        bool is_constant = true;
        bool has_side_effects = false;

        if (node->initial && node->initial->kind() != NodeType::NoOp) {
            auto prev_accepted_type = accepted_list_type;
            accepted_list_type = list_type->tp;
            auto newValue = visit(node->initial);
            accepted_list_type = prev_accepted_type;

            if (!newValue.type) {
                throw Exceptions::ValueException(
                    LST_INIT_MUST_BE_VALID_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            if (newValue.type->kind != Interpreting::SymbolType::ListType) {
                throw Exceptions::TypeException(
                    INVALID_LIST_INIT_NOT_LIST_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            if (!counts_as(newValue.type->tp, list_type->tp)) {
                throw Exceptions::TypeException(
                    INVALID_LST_DECL_TYPE_EXCP + list_type->tp->name + WITH_LST_VAL_OF_TYPE_EXCP + newValue.type->tp->name,
                    node->line_number,
                    node->column_number
                );
            }

            was_init = true;
            is_constant = newValue.is_constant;
            has_side_effects = newValue.has_side_effects;
        }

        auto in_table = currentScope->addSymbol({
            list_type,
            node->name.value
        });

        in_table->is_initialized = was_init;
        in_table->content_is_constant = is_constant;
        in_table->content_has_side_effects = has_side_effects;

        return {};
    }

    NodeResult SemanticAnalyzer::visit_Variable(const std::shared_ptr<Parsing::VariableNode>& node) {
        auto found = currentScope->findSymbol(node->token.value);

        if (found != nullptr) {
                // Check if initialized!
            if (!found->has_been_checked) {
                consume_lazy(found);
            }

            if (found->is_initialized) {
                return {
                    found->tp,
                    found->content_is_constant,
                    found->content_has_side_effects
                };
            } else {
                throw Exceptions::ValueException(
                    USI_VAR_NOT_INIT_EXCP,
                    node->line_number,
                    node->column_number
                );
            }
        } else {
            throw Exceptions::NameException(
                VAR_CALLED_EXCP + node->token.value + NOT_DEFINED_EXCP,
                node->line_number,
                node->column_number
            );
        }
    }

    NodeResult SemanticAnalyzer::visit_Assignment(const std::shared_ptr<Parsing::AssignmentNode>& node) {
        auto varSym = getSymbolFromNode(node->expr);

        if (varSym) {
            if (!varSym->tp || varSym->isType) {
                throw Exceptions::SemanticException(
                    INVALID_ASS_SYMBOL_NOT_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            auto is_list_type = varSym->tp->kind == Interpreting::SymbolType::ListType;
            auto prev_acc_list = accepted_list_type;
            if (is_list_type) {
                accepted_list_type = varSym->tp->tp;
            }
            auto newValue = visit(node->val);
            if (is_list_type) {
                accepted_list_type = prev_acc_list;
            }

            if (!varSym->is_initialized) {
                if (varSym->tp->name == ANY_TP) {
                    varSym->tp = newValue.type;
                }

                varSym->is_initialized = true;
            }

            if (!counts_as(newValue.type, varSym->tp)){
                throw Exceptions::TypeException(
                        INVALID_ASS_TYPE_EXCP + varSym->tp->name + WITH_VAL_OF_TYPE_EXCP + newValue.type->name,
                        node->line_number,
                        node->column_number
                );
            }

            varSym->content_is_constant = newValue.is_constant;
            varSym->content_has_side_effects = newValue.has_side_effects;
        } else {
            throw Exceptions::NameException(
                    ASS_TO_UNKWN_VAR_EXCP,
                    node->line_number,
                    node->column_number
            );
        }
        return {};
    }

    NodeResult SemanticAnalyzer::visit_ListExpression(const std::shared_ptr<Parsing::ListExpressionNode>& node) {
        NodeResult result{};
        bool is_any = false;

        if (node->elements.empty()) {
            auto lst_type = handle_list_type(accepted_list_type ? accepted_list_type : inter.any_type());
            return {lst_type, true, false};
        }

        for (const auto& el : node->elements) {
            auto el_result = visit(el);
            if (!el_result.type) {
                throw Exceptions::TypeException(
                    LST_EL_NO_VALUE_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            if (!result.type) {
                result.type = handle_list_type(el_result.type);
            }
            else if (result.type->tp != el_result.type && !is_any) {
                result.type = inter.globalTable.addListType(inter.any_type());
                is_any = true;
            }

            // If any of the list's elements isn't constant, the list itself isn't
            result.is_constant = result.is_constant && el_result.is_constant;

            // If any of the list's elements has side effects, the list itself has as well
            result.has_side_effects = result.has_side_effects || el_result.has_side_effects;
        }

        return result;
    }

    SemanticAnalyzer::arg_types SemanticAnalyzer::getParamTypes(const std::vector<std::shared_ptr<Node>>& params) {
        arg_types ts;

        auto has_seen_an_optional = false;
        for (const auto& par : params) {
            bool is_optional;
            Interpreting::Symbol* ft;
            switch (par->kind()) {
                case NodeType::VarDeclaration: {
                    auto as_var_declaration_node = Node::as<VarDeclarationNode>(par);
                    ft = getSymbolFromNode(as_var_declaration_node->var_type);
                    if (ft) {
                        is_optional = as_var_declaration_node->initial && as_var_declaration_node->initial->kind() != NodeType::NoOp;
                    } else {
                        // TODO: Handle Error
                        // Error! Unknown type par.type.value
                        throw Exceptions::TypeException(
                                UNKWN_TYPE_EXCP "'.",
                                par->line_number,
                                par->column_number
                        );
                    }
                    break;
                }
                case NodeType::ListDeclaration: {// FIXME: List types are registered as their basetype and not as listtype
                    auto as_var_declaration_node = Node::as<ListDeclarationNode>(par);
                    auto base_type = getSymbolFromNode(as_var_declaration_node->var_type);
                    ft = handle_list_type(base_type, as_var_declaration_node->dim);
                    if (ft) {
                        is_optional = as_var_declaration_node->initial && as_var_declaration_node->initial->kind() != NodeType::NoOp;
                    } else {
                        throw Exceptions::TypeException(
                                UNKWN_TYPE_EXCP "'.",
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
            if (is_optional) {
                if (!has_seen_an_optional){
                    has_seen_an_optional = true;
                }
            } else if (has_seen_an_optional) {
                // Error! Defining a non optional argument after an optional.
                throw Exceptions::SemanticException(
                    CANT_DEFINE_NON_OPT_AFTER_OPT_EXCP,
                    par->line_number,
                    par->column_number
                );
            }
            ts.emplace_back(ft, is_optional);
        }

        return ts;
    }

    NodeResult SemanticAnalyzer::visit_FuncExpression(const std::shared_ptr<Parsing::FuncExpressionNode>& node) {
        auto returnType = inter.any_type();

        Interpreting::SymbolTable func_scope {"func_" ANONYMUS_MSG "_scope", {}, currentScope};
        // Setup the current accepted return type as whatever this one returns.

        auto temp = currentScope;
        currentScope = &func_scope;
        // Store the current function symbol in a variable of SemAn
        for (const auto& par : node->params) {
            visit(par);
            std::string name;
            if (par->kind() == NodeType::VarDeclaration) {
                name = Node::as<VarDeclarationNode>(par)->name.value;
            } else {
                name = Node::as<ListDeclarationNode>(par)->name.value;
            }
            currentScope->findSymbol(name)->is_initialized = true;
        }

        auto prev_ret = accepted_return_type;
        accepted_return_type = returnType;
        auto could_return = can_return;
        can_return = true;

        visit(node->body);

        if (accepted_return_type != returnType) {
            returnType = accepted_return_type;
            node->retType = VariableNode::create(Lexing::Token(Lexing::TokenType::ID, returnType->name));
        } else {
            returnType = nullptr;
        }

        can_return = could_return;
        accepted_return_type = prev_ret;
        currentScope = temp;

        auto paramTypes = getParamTypes(node->params);

        auto typeName = Interpreting::Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalScope.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalScope.addFuncType(returnType, paramTypes);
            add_semantic_context(typeOfFunc, func_scope);
            functions_context.insert(std::pair(typeOfFunc, std::move(paramTypes)));

            typeOfFunc->ondestruction = nullptr;
        }

        return {typeOfFunc};
    }

    NodeResult SemanticAnalyzer::visit_FuncDecl(const std::shared_ptr<Parsing::FuncDeclNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
                node->line_number,
                node->column_number
            );
        }

        Interpreting::Symbol* returnType;

        if (node->retType->kind() == Parsing::NodeType::NoOp) {
            returnType = nullptr;
        } else {
            returnType = getSymbolFromNode(node->retType);
        }

        auto paramTypes = getParamTypes(node->params);

        auto typeName = Interpreting::Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalScope.findSymbol(typeName);

        Interpreting::SymbolTable func_scope;

        if (!typeOfFunc) {
            typeOfFunc = globalScope.addFuncType(returnType, typeName);
            func_scope = *add_function_semantic_context(typeOfFunc, typeName, paramTypes);
        } else {
            auto existing_context = get_semantic_context(typeOfFunc);
            if (existing_context) {
                func_scope = *existing_context;
            } else {
                func_scope = *add_function_semantic_context(typeOfFunc, typeName, paramTypes);
            }
        }
        func_scope.setParent(currentScope);

        auto func_symbol = currentScope->addSymbol({
            .tp=typeOfFunc,
            .name=node->name.value,
            .kind=Interpreting::SymbolType::FunctionSymbol
        });
        func_symbol->is_initialized = true;

        auto temp = currentScope;
        currentScope = &func_scope;
        for (const auto& par : node->params) {
            visit(par);
            std::string name;
            if (par->kind() == NodeType::VarDeclaration) {
                name = Node::as<VarDeclarationNode>(par)->name.value;
            } else {
                name = Node::as<ListDeclarationNode>(par)->name.value;
            }
            currentScope->findSymbol(name)->is_initialized = true;
        }

        // FIXME: Make sure that every branch returns.
        //  I think maybe adding a field fpr that along with can_return.
        //  And branching nodes like if or loop check if their branches return.

        // This is a little messy.
        // I don't like doing this kind of error checking inside of the whole module.
        // But the fact that I just bubble it up means it probably won't change much.
        add_lazy_check(func_symbol,{
            [this, body=node->body, func_scope, returnType](auto){
                auto prev_accepted = accepted_return_type;
                auto could_return = can_return;
                can_return = true;
                accepted_return_type = returnType;
                auto temp = currentScope;
                currentScope = const_cast<Interpreting::SymbolTable*>(&func_scope);
                visit(body);
                currentScope = temp;

                accepted_return_type = prev_accepted;
                can_return = could_return;
            },
            temp
        });

        // try {
        //     visit(node->body);
        // } catch (Exceptions::OdoException& e) {
        //     temp->removeSymbol(func_symbol);
        //     throw e;
        // }
        currentScope = temp;
        // accepted_return_type = prev_accepted;
        // can_return = could_return;

        return {};
    }

    NodeResult SemanticAnalyzer::visit_FuncCall(const std::shared_ptr<Parsing::FuncCallNode>& node) {
        if (node->fname.tp != Lexing::NOTHING) {
            auto in_natives = native_function_data.find(node->fname.value);
            if (in_natives != native_function_data.end()) {
                for(const auto& arg : node->args) {
                    if (!visit(arg).type) {
                        throw Exceptions::ValueException(
                            INVALID_CALL_ARG_EXCP,
                            arg->line_number,
                            arg->column_number
                        );
                    }
                }

                return in_natives->second;
            }
        }

        auto fVal = visit(node->expr);
        if (!fVal.type) {
            // Error! Not a valid function
            throw Exceptions::TypeException(
                UNEXP_CALL_NOT_FUNC_EXCP,
                node->line_number,
                node->column_number
            );
        }
        auto functionType = fVal.type;

        if (fVal.type->kind == Interpreting::SymbolType::FunctionType) {
            const auto& parameters_in_template = get_function_semantic_context(functionType);
            auto& call_args = node->args;

            if (call_args.size() > parameters_in_template.size()) {
                // Error! Function of type [type name] takes [parameters_in_template] arguments, but was called with [call_args.size()]
                throw Exceptions::SemanticException(
                    FUNC_OF_TP_EXCP + functionType->name + TAKES_EXCP + std::to_string(parameters_in_template.size()) + ARGS_BUT_CALLED_EXCP + std::to_string(call_args.size()),
                    node->line_number,
                    node->column_number
                );
            }

            for (size_t i = 0; i < parameters_in_template.size(); i++) {
                auto param_def = parameters_in_template[i];
                auto par = param_def.first;

                if (call_args.size() > i) {
                    const auto& argument = call_args.at(i);
                    auto arg_result = visit(argument);

                    auto is_empty_list = false;
                    if (par->kind == Interpreting::SymbolType::ListType &&
                        argument->kind() == Parsing::NodeType::ListExpression) {
                        is_empty_list = Node::as<ListExpressionNode>(argument)->elements.empty();
                    }
                    if (!is_empty_list && !counts_as(arg_result.type, par)) {
                        // Error! invalid type for call argument
                        throw Exceptions::TypeException(
                                INVALID_TP_FOR_ARG_EXCP + std::to_string(i) + EXPC_TP_EXCP + par->name + BUT_RECVD_EXCP + arg_result.type->name,
                                node->line_number,
                                node->column_number
                        );
                    }
                } else if (!param_def.second) {
                    // Error! No value for function call argument
                    throw Exceptions::SemanticException(
                            NO_VAL_FOR_FUNC_ARG_EXCP + std::to_string(i),
                            node->line_number,
                            node->column_number
                    );
                }
            }

            return {functionType->tp};
        }

        throw Exceptions::ValueException(
            VAL_NOT_FUNC_EXCP,
            node->line_number,
            node->column_number
        );
    }

    NodeResult SemanticAnalyzer::visit_FuncBody(const std::shared_ptr<Parsing::FuncBodyNode>& node) {
        auto temp = currentScope;
        auto bodyScope = Interpreting::SymbolTable("func-body-scope", {}, currentScope);

        currentScope = &bodyScope;

        for (const auto& st : node->statements) {
            visit(st);
        }

        currentScope = temp;
        return {};
    }

    NodeResult SemanticAnalyzer::visit_Return(const std::shared_ptr<Parsing::ReturnNode>& node) {
        if (!can_return) {
            // Error! return statement outside of a function body.
            throw Exceptions::SemanticException(
                RET_OUTS_FUNC_BODY_EXCP,
                node->line_number,
                node->column_number
            );
        }

        auto result = visit(node->val);

        if (accepted_return_type == inter.any_type()) {
            accepted_return_type = result.type;
        } else if (accepted_return_type != result.type) {
            if (accepted_return_type == nullptr) {
                // Error! returning a value inside a void function.
                throw Exceptions::SemanticException(
                    RET_ON_VOID_FUNC_EXCP,
                    node->line_number,
                    node->column_number
                );
            }
            if (!counts_as(result.type, accepted_return_type)) {
                // Error! The returned value does not match the functions return type of accepted_return_type->name
                throw Exceptions::TypeException(
                    RET_VAL_TYPE_EXCP + (result.type ? result.type->name : "void") + NOT_MATCH_FUNC_EXCP + accepted_return_type->name + "'.",
                    node->line_number,
                    node->column_number
                );
            }
        }
        return {};
    }

    NodeResult SemanticAnalyzer::visit_Enum(const std::shared_ptr<Parsing::EnumNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
                node->line_number,
                node->column_number
            );
        }

        auto* enumInTable = currentScope->addSymbol({
            .name=node->name.value,
            .isType=true,
            .kind=Interpreting::SymbolType::EnumType
        });

        auto enum_variant_scope = add_semantic_context(enumInTable, "enum_" + node->name.value + "_scope");

        for (const auto& variant : node->variants) {
            // By the parsing module, this can't not be a variable

            // But if you hand construct the tree, I should test for that.
            // TODO: Don't trust this conversion.
            auto as_variable = Node::as<VariableNode>(variant);
            auto variant_name = as_variable->token.value;

            auto variant_in_table = enum_variant_scope->addSymbol({
                .tp=enumInTable,
                .name=variant_name,
                .kind=Interpreting::SymbolType::EnumSymbol
            });
            variant_in_table->is_initialized = true;
        }

        enumInTable->is_initialized = true;

        return {};
    }

    NodeResult SemanticAnalyzer::visit_Class(const std::shared_ptr<Parsing::ClassNode>& node) {
        Interpreting::Symbol* typeSym = nullptr;

        if (node->ty->kind() != Parsing::NodeType::NoOp) {
            auto sym = getSymbolFromNode(node->ty);
            if (!sym || !sym->isType || sym->kind != Interpreting::SymbolType::ClassType) {
                throw Exceptions::TypeException(
                    CLASS_MUST_INH_TYPE_EXCP /* NO NAME! */ IS_INVALID_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            typeSym = sym;
        }

        Interpreting::Symbol newClassSym = {
                .tp=typeSym,
                .name=node->name.value,
                .isType = true,
                .kind = Interpreting::SymbolType::ClassType,
        };
        auto inTable = currentScope->addSymbol(newClassSym);

        auto classScope = add_semantic_context(inTable, "class-" + node->name.value + "-scope");

        // Why would the body be empty?
        add_lazy_check(inTable, {
            [this, inTable, node, classScope](Interpreting::SymbolTable* parentScope){
                push_lazy_scope();
                auto prevScope = currentScope;
                currentScope = classScope;
                visit(node->body);
                currentScope = parentScope;

                Interpreting::SymbolTable instance_scope_temporary {"instance_"+node->name.value+"_scope", {}, currentScope};

                auto inst_template_name = "__$" + inTable->name + "_instance_template";
                Interpreting::Symbol inst_template {
                    inTable,
                    inst_template_name
                };
                inst_template.is_initialized = true;

                auto instanceInTable = currentScope->addSymbol(inst_template);
                auto instance_scope = add_semantic_context(instanceInTable, instance_scope_temporary);

                instance_scope->addSymbol({
                    .tp=inTable,
                    .name=THIS_VAR,
                    .is_initialized=true
                });
                auto instance_body_node = InstanceBodyNode::create(Node::as<ClassBodyNode>(node->body)->statements);

                std::vector<Interpreting::SymbolTable*> inheritedScopes{instance_scope};

                auto currentClass = inTable;
                while (currentClass->tp != nullptr) {
                    auto upperClass = currentClass->tp;
                    if (!upperClass->has_been_checked)
                        consume_lazy(upperClass);

                    currentClass = upperClass;

                    auto this_template_name = "__$" + currentClass->name + "_instance_template";
                    auto currentClassInstanceTemplate = currentScope->findSymbol(this_template_name);

                    auto currentInstanceScope = get_semantic_context(currentClassInstanceTemplate);

                    auto inherScope =new Interpreting::SymbolTable{
                            "inherited-scope-" + currentClass->name,
                            currentInstanceScope->symbols
                    };

                    inherScope->setParent(inheritedScopes.back()->getParent());
                    inheritedScopes.back()->setParent(inherScope);
                    inheritedScopes.push_back(inherScope);
                }

                instance_contexts[instanceInTable] = std::move(inheritedScopes);
                instanceInTable->ondestruction = [&] (auto sym) {
                    if(!deactivate_cleanup) {
                        instance_contexts.erase(sym);
                    }
                };

                add_lazy_check(instanceInTable, {
                    [this, instance_scope, instance_body_node](auto) {
                        // Make sure to check everything in the instance when creating it.
                        push_lazy_scope();
                        auto temp = currentScope;
                        currentScope = instance_scope;
                        visit(instance_body_node);
                        currentScope = temp;
                        pop_lazy_scope();
                    },
                    currentScope
                });

                currentScope = prevScope;
                pop_lazy_scope();
            },
            currentScope
        });

//        try {
//            visit(node->body);
//        } catch (Exceptions::OdoException& e) {
//            prevScope->removeSymbol(inTable);
//            throw e;
//        }

        inTable->is_initialized = true;

        // Generate the instance template and visit the body

        return {};
    }

    NodeResult SemanticAnalyzer::visit_ClassBody(const std::shared_ptr<Parsing::ClassBodyNode>& node) {
        for (auto& st : node->statements) {
            if (st->kind() == NodeType::StaticStatement)
                visit(Node::as<StaticStatementNode>(st)->statement);
        }

        return {};
    }

    NodeResult SemanticAnalyzer::visit_ConstructorDecl(const std::shared_ptr<Parsing::ConstructorDeclNode>& node) {
        auto paramTypes = getParamTypes(node->params);

        auto typeName = Interpreting::Symbol::constructFuncTypeName(nullptr, paramTypes);

        auto typeOfFunc = globalScope.findSymbol(typeName);

        Interpreting::SymbolTable func_scope;

        if (!typeOfFunc) {
            typeOfFunc = globalScope.addFuncType(nullptr, typeName);
            func_scope = *add_function_semantic_context(typeOfFunc, typeName, paramTypes);
        } else {
            auto existing_context = get_semantic_context(typeOfFunc);
            if (existing_context) {
                func_scope = *existing_context;
            } else {
                func_scope = *add_function_semantic_context(typeOfFunc, typeName, paramTypes);
            }
        }
        func_scope.setParent(currentScope);

        auto func_symbol = currentScope->addSymbol({
            .tp=typeOfFunc,
            .name="constructor",
            .kind=Interpreting::SymbolType::FunctionSymbol
        });
        func_symbol->is_initialized = true;

        auto temp = currentScope;
        currentScope = &func_scope;
        for (const auto& par : node->params) {
            visit(par);
            std::string name;
            if (par->kind() == NodeType::VarDeclaration) {
                name = Node::as<VarDeclarationNode>(par)->name.value;
            } else {
                name = Node::as<ListDeclarationNode>(par)->name.value;
            }
            currentScope->findSymbol(name)->is_initialized = true;
        }
        currentScope = temp;

        // This is a little messy.
        // I don't like doing this kind of error checking inside of the whole module.
        // But the fact that I just bubble it up means it probably won't change much.
        add_lazy_check(func_symbol, {
            [this, body=node->body, func_scope](auto){
                auto temp = currentScope;
                currentScope = const_cast<Interpreting::SymbolTable*>(&func_scope);
                auto prev_accepted = accepted_return_type;
                auto could_return = can_return;
                can_return = true;
                accepted_return_type = nullptr;
                visit(body);
                currentScope = temp;
                accepted_return_type = prev_accepted;
                can_return = could_return;
            },
            temp
        });

        // try {
        //     visit(node->body);
        // } catch (Exceptions::OdoException& e) {
        //     temp->removeSymbol(func_symbol);
        //     throw e;
        // }
        // currentScope = temp;
        // accepted_return_type = prev_accepted;
        // can_return = could_return;

        return {};
    }

    NodeResult SemanticAnalyzer::visit_InstanceBody(const std::shared_ptr<Parsing::InstanceBodyNode>& node) {
        for (auto& st : node->statements) {
            if (st->kind() != NodeType::StaticStatement)
                visit(st);
        }

        return {};
    }

    NodeResult SemanticAnalyzer::visit_ClassInitializer(const std::shared_ptr<Parsing::ClassInitializerNode>& node) {
        auto class_symbol = getSymbolFromNode(node->cls);
        if (!class_symbol) {
            // Error! Unknown type node->name.value
            throw Exceptions::NameException(
                    // TODO: Get the class' name
                    UNKNWN_CLASS_EXCP "?'.",
                    node->line_number,
                    node->column_number
            );
        } else if (!class_symbol->has_been_checked) {
            consume_lazy(class_symbol);
        }

        auto template_name = "__$" + class_symbol->name + "_instance_template";
        Interpreting::Symbol* instance_template;
        if (node->cls->kind() == Parsing::NodeType::StaticVar) {
            auto as_static = Node::as<StaticVarNode>(node->cls);
            auto instance_getter = Parsing::StaticVarNode::create(as_static->inst, {Lexing::ID, template_name});
            instance_template = getSymbolFromNode(instance_getter);
        } else {
            instance_template = currentScope->findSymbol(template_name);
        }

        // Annoying to have to put this in every symbol check.
        if (!instance_template->has_been_checked) {
            consume_lazy(instance_template);
        }

        auto instance_scope = get_semantic_context(instance_template);

//        auto constr_name = VariableNode::create({Lexing::ID, "constructor"});
//
//        auto constr_call = FuncCallNode::create(constr_name, {Lexing::NOTHING, ""}, node->params);

        auto constr = instance_scope->findSymbol("constructor");

        if (!constr) {
            if (node->params.empty()) return {class_symbol};
            throw Exceptions::SemanticException(
                CONSTR_CALL_TAKES_EXCP "0" ARGS_BUT_CALLED_EXCP + std::to_string(node->params.size()),
                node->line_number,
                node->column_number
            );
        }
        if (!constr->has_been_checked)
            consume_lazy(constr);

        const auto& parameters_in_template = get_function_semantic_context(constr->tp);
        auto& call_args = node->params;

        if (call_args.size() > parameters_in_template.size()) {
            throw Exceptions::SemanticException(
                    CONSTR_CALL_TAKES_EXCP + std::to_string(parameters_in_template.size()) + ARGS_BUT_CALLED_EXCP + std::to_string(call_args.size()),
                    node->line_number,
                    node->column_number
            );
        }

        for (size_t i = 0; i < parameters_in_template.size(); i++) {
            auto param_def = parameters_in_template[i];
            auto par = param_def.first;

            if (call_args.size() > i) {
                const auto& argument = call_args.at(i);
                auto arg_result = visit(argument);

                if (!counts_as(arg_result.type, par)) {
                    // Error! invalid type for call argument
                    throw Exceptions::TypeException(
                            INVALID_TP_FOR_CONSTR_ARG_EXCP + std::to_string(i) + EXPC_TP_EXCP + par->name + BUT_RECVD_EXCP + arg_result.type->name,
                            node->line_number,
                            node->column_number
                    );
                }
            } else if (!param_def.second) {
                // Error! No value for function call argument
                throw Exceptions::SemanticException(
                        NO_VAL_FOR_CONSTR_ARG_EXCP + std::to_string(i),
                        node->line_number,
                        node->column_number
                );
            }
        }

        return {class_symbol};
    }

    NodeResult SemanticAnalyzer::visit_MemberVar(const std::shared_ptr<Parsing::MemberVarNode>& node) {
        auto instance = visit(node->inst);

        if (!instance.type || instance.type->kind != Interpreting::SymbolType::ClassType) {
            throw Exceptions::ValueException(
                INVALID_INS_MEM_OP_EXCP,
                node->line_number,
                node->column_number
            );
        }

        auto template_name = "__$" + instance.type->name + "_instance_template";
        auto parentScope = instance.type->table;
        auto instance_template = parentScope->findSymbol(template_name);

        auto instance_scopes = instance_contexts.at(instance_template);

        for (auto tab : instance_scopes) {
            auto foundSymbol = tab->findSymbol(node->name.value, false);
            if (foundSymbol) {
                return {foundSymbol->tp, foundSymbol->content_is_constant, foundSymbol->content_has_side_effects};
            }
        }

        throw Exceptions::NameException(
                NO_MEM_CALLED_EXCP + node->name.value + IN_THE_INST_EXCP,
                node->line_number,
                node->column_number
        );
    }

    NodeResult SemanticAnalyzer::visit_StaticVar(const std::shared_ptr<Parsing::StaticVarNode>& node) {
        auto symbol = getSymbolFromNode(node);
        if (symbol) {
            return {
                symbol->tp,
                true,
                false
            };
        } else {
            throw Exceptions::NameException(
                NO_STATIC_CALLED_EXCP + node->name.value + IN_CLASS_EXCP,
                node->line_number,
                node->column_number
            );
        }
    }

    NodeResult SemanticAnalyzer::visit_Module(const std::shared_ptr<Parsing::ModuleNode>& node) {
        auto module_in_table = currentScope->addSymbol({
            .tp=nullptr,
            .name=node->name.value,
            .kind=Interpreting::SymbolType::ModuleSymbol
        });

        auto module_context = add_semantic_context(module_in_table, "module_" + module_in_table->name + "_scope");

        module_in_table->is_initialized = true;

        add_lazy_check(module_in_table, {
            [this, node, module_context](auto){
                auto temp = currentScope;
                currentScope = module_context;
                push_lazy_scope();
                for (const auto& st : node->statements) {
                    visit(st);
                }
                pop_lazy_scope();
                currentScope = temp;
            },
            currentScope
        });

        return {};
    }

    NodeResult SemanticAnalyzer::visit_Import(const std::shared_ptr<Parsing::ImportNode>& node) {
        analyze_as_module(node->path.value, node->name);
        return {};
    }

    NodeResult SemanticAnalyzer::visit_Define(const std::shared_ptr<Parsing::DefineNode>& node) {
        std::vector<std::pair<Interpreting::Symbol*, bool>> as_function_types;
        as_function_types.reserve(node->args.size());

        for(const auto& sm : node->args) {
            Interpreting::Symbol* found_symbol;

            if (sm.first.value.ends_with("[]")) {
                found_symbol = string_to_list_type(sm.first.value);
            } else {
                found_symbol = currentScope->findSymbol(sm.first.value);
                if (!found_symbol) {
                    throw Exceptions::NameException(
                        UNKNWN_TP_IN_FUNC_DEF_EXCP + sm.first.value,
                        node->line_number,
                        node->column_number
                    );
                }

                if (!found_symbol->isType) {
                    throw Exceptions::TypeException(
                        SYM_IN_FUNC_DEF_EXCP + sm.first.value + IS_NOT_TP_EXCP,
                        node->line_number,
                        node->column_number
                    );
                }
            }

            as_function_types.push_back({found_symbol, sm.second});
        }

        Interpreting::Symbol* ret_type_symbol{nullptr};
        if (node->retType.tp != Lexing::NOTHING) {
            ret_type_symbol = currentScope->findSymbol(node->retType.value);

            if (!ret_type_symbol) {
                throw Exceptions::NameException(
                    UNKNWN_TP_IN_FUNC_DEF_EXCP + node->retType.value,
                    node->line_number,
                    node->column_number
                );
            }

            if (!ret_type_symbol->isType) {
                throw Exceptions::TypeException(
                    SYM_IN_FUNC_DEF_EXCP + node->retType.value + IS_NOT_TP_EXCP,
                    node->line_number,
                    node->column_number
                );
            }
        }

        auto already_in = currentScope->symbolExists(node->name.value);
        if (already_in) {
            throw Exceptions::NameException(
                SYM_CALLED_EXCP + node->name.value + ALR_EXISTS_IN_SCOPE_EXCP
            );
        }

        auto fnName = Interpreting::Symbol::constructFuncTypeName(ret_type_symbol, as_function_types);
        auto fnType = globalScope.findSymbol(fnName);

        Interpreting::SymbolTable func_scope;
        if (!fnType) {
            fnType = globalScope.addFuncType(ret_type_symbol, fnName);
            func_scope = *add_function_semantic_context(fnType, fnName, as_function_types);
        } else {
            auto existing_context = get_semantic_context(fnType);
            if (existing_context) {
                func_scope = *existing_context;
            } else {
                func_scope = *add_function_semantic_context(fnType, fnName, as_function_types);
            }
        }

        currentScope->addAlias(node->name.value, fnType);

        return {};
    }

    void SemanticAnalyzer::analyze_as_module(const std::string& path, const Lexing::Token& name) {
        bool has_extension = ends_with(path, ".odo");

        std::string full_path = path;
        if (!has_extension)
            full_path += ".odo";
        auto filename = io::get_file_name(full_path, true);

        if (name.tp != Lexing::NOTHING)
            filename = name.value;

        if (currentScope->findSymbol(filename)) {
            throw Exceptions::NameException(
                SYM_CALLED_EXCP + filename + ALR_EXISTS_IN_SCOPE_EXCP
            );
        }

        std::string code;
        try {
            code = io::read_file(full_path);
        } catch (Exceptions::IOException&) {
            std::string msg = CANNOT_IMPORT_MODULE_EXCP + full_path + "'.";
            throw Exceptions::FileException(msg);
        }
        Parsing::Parser pr;
        pr.set_text(code);

        auto body = pr.program_content();

        auto file_module = ModuleNode::create(
            Lexing::Token(Lexing::STR, filename),
            body
        );

        // TODO: Add some sort of callstack.
        visit(file_module);
    }

    NodeResult SemanticAnalyzer::from_repl(const std::shared_ptr<Parsing::Node> & node) {
        auto temp_scope = currentScope;
        currentScope = &replScope;

        auto result = visit(node);

        currentScope = temp_scope;
        return result;
    }
}