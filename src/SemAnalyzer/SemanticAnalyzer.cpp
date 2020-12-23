//
// Created by Luis Gonzalez on 12/19/20.
//

#include "SemAnalyzer/SemanticAnalyzer.h"

#include <utility>
#include "Exceptions/exception.h"
#include "Interpreter/Interpreter.h"

#define TEST_SEMANTICS

namespace Odo::Semantics {
    using namespace Parsing;
    // This should initialize the symbol tables and basic types, like the Interpreter.
    SemanticAnalyzer::SemanticAnalyzer(Interpreting::Interpreter& inter_): inter(inter_) {
        currentScope = &inter.globalTable;
        replScope = { "repl-analyzer", {}, &inter.globalTable };
    }

    Interpreting::SymbolTable* SemanticAnalyzer::add_semantic_context(Interpreting::Symbol* sym, std::string name) {
        semantic_contexts.insert(std::pair(sym, Interpreting::SymbolTable{
            std::move(name),
            {},
            currentScope
        }));

        sym->ondestruction = [this](auto* sym){
            to_clean.push_back(sym);
        };

        return &semantic_contexts.find(sym)->second;
    }

    Interpreting::SymbolTable* SemanticAnalyzer::get_semantic_context(Interpreting::Symbol* sym) {
        auto in_my_map = semantic_contexts.find(sym);
        if (in_my_map != semantic_contexts.end()) return &in_my_map->second;

        return nullptr;
    }

    void SemanticAnalyzer::clean_contexts() {
        // This is what used to add the most slowdown when I had aa value table. I'll test it out a few times when I get every thing working.
        for (auto el : to_clean) {
            semantic_contexts.erase(semantic_contexts.find(el));
        }
    }

    bool SemanticAnalyzer::counts_as(Interpreting::Symbol* type1, Interpreting::Symbol* type2) {
        if (type2 == inter.any_type()) return true;

        if (type1->is_numeric() && type2->is_numeric()) return true;

        auto curr = type1;
        while (curr) {
            if (curr == type2) return true;

            curr = curr->tp;
        }

        return false;
    }

    Interpreting::Symbol* SemanticAnalyzer::getSymbolFromNode(const std::shared_ptr<Parsing::Node>& mem) {
        Interpreting::Symbol* varSym = nullptr;

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
                                "(SemAn) " "'" + leftHandSym->name + NOT_VALID_INST_EXCP,
                                mem->line_number,
                                mem->column_number
                        );
                    }

                    // TODO: Implement SemanticContexts
                    // varSym = as_instance_value->ownScope.findSymbol(as_member_node->name.value);
                } else {
                    throw Exceptions::ValueException(
                            "(SemAn) " SYM_NOT_DEFINED_EXCP,
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
//                    if (theValue->kind() == ValueType::ModuleVal) {
//                        varSym = Value::as<ModuleValue>(theValue)->ownScope.findSymbol(as_static_var->name.value, false);
//                    } else if (theValue->kind() == ValueType::ClassVal) {
//                        varSym = Value::as<ClassValue>(theValue)->getStaticVarSymbol(as_static_var->name.value);
//                    } else if (theValue->kind() == ValueType::InstanceVal) {
//                        varSym = Value::as<InstanceValue>(theValue)->getStaticVarSymbol(as_static_var->name.value);
//                    } else
                    if (leftHandSym->kind == Interpreting::SymbolType::EnumType) {
                        auto context = get_semantic_context(leftHandSym);
                        auto sm = context->findSymbol(as_static_var->name.value, false);
                        if (sm) {
                            return sm;
                        } else {
                            throw Exceptions::NameException(
                                    "'" + as_static_var->name.value + NOT_VARIANT_IN_ENUM_EXCP,
                                    mem->line_number,
                                    mem->column_number
                            );
                        }
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
                            "(SemAn) " NO_VALUE_TO_INDEX_EXCP,
                            as_index_node->line_number,
                            as_index_node->column_number
                    );
                }

                if (visited_source.type->kind == Interpreting::SymbolType::ListType) {
                    auto visited_indx = visit(as_index_node->expr);

                    if (!visited_indx.type) {
                        throw Exceptions::ValueException(
                                "(SemAn) " INDEX_MUST_BE_VALID_EXCP,
                                as_index_node->expr->line_number,
                                as_index_node->expr->column_number
                        );
                    }

                    if (visited_indx.type->name == INT_TP) {
                        auto element_template_name = "__" + visited_source.type->name + "_list_element";

                        varSym = currentScope->findSymbol(element_template_name);
                    } else {
                        throw Exceptions::TypeException(
                                "(SemAn) " LST_ONLY_INDX_NUM_EXCP,
                                as_index_node->line_number,
                                as_index_node->column_number
                        );
                    }
                } else {
                    throw Exceptions::ValueException(
                            "(SemAn) " ASS_TO_INVALID_INDX_EXCP,
                            as_index_node->line_number,
                            as_index_node->column_number
                    );
                }
                break;
            }
            default:
                break;
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
                // breaking = true;
                /* ToRemoveLater */ break;
                // return null;
            case NodeType::Continue:
                // continuing = true;
                /* ToRemoveLater */ break;
                // return null;
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
                // return visit_FuncExpression(Node::as<FuncExpressionNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::FuncDecl:
                // return visit_FuncDecl(Node::as<FuncDeclNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::FuncCall:
                // return visit_FuncCall(Node::as<FuncCallNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::FuncBody:
                // return visit_FuncBody(Node::as<FuncBodyNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::Return:
                // return visit_Return(Node::as<ReturnNode>(node));
                /* ToRemoveLater */ break;

            case NodeType::Enum:
                 return visit_Enum(Node::as<EnumNode>(node));

                // Classes
            case NodeType::Class:
                // return visit_Class(Node::as<ClassNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::ClassBody:
                // return visit_ClassBody(Node::as<ClassBodyNode>(node));
                /* ToRemoveLater */ break;
//          Broken or incomplete.
            case NodeType::ConstructorDecl:
                // return visit_ConstructorDecl(Node::as<ConstructorDeclNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::ConstructorCall:
                // return visit_ConstructorCall(Node::as<ConstructorCallNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::InstanceBody:
                // return visit_InstanceBody(Node::as<InstanceBodyNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::ClassInitializer:
                // return visit_ClassInitializer(Node::as<ClassInitializerNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::StaticStatement:
                // throw Exceptions::OdoException(
                /* ToRemoveLater */ break;
//                        STATIC_ONLY_CLASS_EXCP,
//                        node->line_number,
//                        node->column_number
//                );
            case NodeType::MemberVar:
                // return visit_MemberVar(Node::as<MemberVarNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::StaticVar:
                 return visit_StaticVar(Node::as<StaticVarNode>(node));

            case NodeType::Module:
                // return visit_Module(Node::as<ModuleNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::Import:
                // return visit_Import(Node::as<ImportNode>(node));
                /* ToRemoveLater */ break;

            case NodeType::Debug:
                // noop;
            case NodeType::Null:
                return {inter.get_null()->type, true, false};
        }

#ifdef TEST_SEMANTICS
        NOT_IMPLEMENTED("Semantics");
#endif
        return {};
    }

    NodeResult SemanticAnalyzer::visit_Double(const std::shared_ptr<Parsing::DoubleNode>& node) {
        return {inter.globalTable.findSymbol(DOUBLE_TP), true, false};
    }

    NodeResult SemanticAnalyzer::visit_Int(const std::shared_ptr<Parsing::IntNode>& node) {
        return {inter.globalTable.findSymbol(INT_TP), true, false};
    }

    NodeResult SemanticAnalyzer::visit_Str(const std::shared_ptr<Parsing::StrNode>& node) {
        return {inter.globalTable.findSymbol(STRING_TP), true, false};
    }

    NodeResult SemanticAnalyzer::visit_Bool(const std::shared_ptr<Parsing::BoolNode>& node) {
        return {inter.globalTable.findSymbol(BOOL_TP), true, false};
    }

    NodeResult SemanticAnalyzer::visit_BinOp(const std::shared_ptr<Parsing::BinOpNode>& node) {
        auto leftVisited = visit(node->left);
        auto rightVisited = visit(node->right);

        if (!leftVisited.type) {
            //Error! Left operand in binary operation has no value.
            throw Exceptions::ValueException(
                "(SemAn) " LEFT_OP_BIN_NO_VAL_EXCP,
                node->line_number,
                node->column_number
            );
        }

        if (!rightVisited.type) {
            //Error! Right operand in binary operation has no value.
            throw Exceptions::ValueException(
                "(SemAn) " RIGHT_OP_BIN_NO_VAL_EXCP,
                node->line_number,
                node->column_number
            );
        }

        NodeResult double_result = {inter.globalTable.findSymbol(DOUBLE_TP), true, false};
        NodeResult int_result = {inter.globalTable.findSymbol(INT_TP), true, false};
        NodeResult string_result = {inter.globalTable.findSymbol(STRING_TP), true, false};
        NodeResult bool_result = {inter.globalTable.findSymbol(BOOL_TP), true, false};

        auto both_numerical = leftVisited.type->is_numeric() && rightVisited.type->is_numeric();
        auto same_types = leftVisited.type == rightVisited.type;

        switch (node->token.tp) {
            case Lexing::PLUS: {
                if (leftVisited.type->kind == Interpreting::SymbolType::ListType) {
                    auto element_template_name = "__" + leftVisited.type->name + "_list_element";
                    auto template_symbol = currentScope->findSymbol(element_template_name);

                    if (rightVisited.type->kind == Interpreting::SymbolType::ListType) {
                        if (!counts_as(rightVisited.type->tp, leftVisited.type->tp)) {
                            // Error! Contenating lists with incompatible types.
                            throw Exceptions::TypeException(
                                "(SemAn) " CONC_LST_INCOM_TPS_EXCP,
                                node->line_number,
                                node->column_number
                            );
                        }
                    } else {
                        if (!counts_as(rightVisited.type, leftVisited.type->tp)) {
                            // Error! Appending to list a symbol of incompatible type.
                            throw Exceptions::TypeException(
                                "(SemAn) " APP_LST_INCOM_TP_EXCP,
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
                    return string_result;
                } else if (both_numerical) {
                    if (same_types && leftVisited.type->name == INT_TP) {
                        return int_result;
                    }

                    return double_result;
                } else {
                    throw Exceptions::TypeException(
                            "(SemAn) " ADD_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            }
            case Lexing::MINUS:
                if (both_numerical) {
                    if (same_types && leftVisited.type->name == INT_TP) {
                        return int_result;
                    }

                    return double_result;
                } else {
                    throw Exceptions::TypeException(
                            "(SemAn) " SUB_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            case Lexing::MUL: {
                if (both_numerical) {
                    if (same_types && leftVisited.type->name == INT_TP) {
                        return int_result;
                    }

                    return double_result;
                } else if (leftVisited.type->kind == Interpreting::SymbolType::ListType) {
                    if (rightVisited.type->name != INT_TP) {
                        // Error! List can only be multiplied with ints.
                        throw Exceptions::TypeException(
                            "(SemAn) " LST_ONLY_MUL_INT_EXCP,
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
                            "(SemAn) " STR_ONLY_MUL_INT_EXCP,
                            node->line_number,
                            node->column_number
                        );
                    }
                    return string_result;
                } else {
                    throw Exceptions::TypeException(
                            "(SemAn) " MUL_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            }
            case Lexing::DIV: {
                if (both_numerical) {
                    // Just return double, even if both are int.
                    // TODO: I need to update the interpreter with that.
                    return double_result;
                } else {
                    throw Exceptions::TypeException(
                            "(SemAn) " DIV_ONLY_DOB_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            }
            case Lexing::MOD:
                if (same_types && leftVisited.type->name == INT_TP) {
                    return int_result;
                } else {
                    throw Exceptions::TypeException(
                            "(SemAn) " MOD_ONLY_INT_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            case Lexing::POW:
                if (both_numerical) {
                    if (same_types && leftVisited.type->name == "int") {
                        return int_result;
                    }

                    return double_result;
                } else {
                    throw Exceptions::TypeException(
                            "(SemAn) " POW_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            case Lexing::EQU:
            case Lexing::NEQ:
                if (same_types || leftVisited.type == inter.null->type || rightVisited.type == inter.null->type) {
                    return bool_result;
                }

                // Else, fallback
            case Lexing::LT:
            case Lexing::GT:
            case Lexing::LET:
            case Lexing::GET:
                if (both_numerical) {
                    return bool_result;
                } else {
                    throw Exceptions::TypeException(
                            "(SemAn) " COM_ONLY_SAME_TP_EXCP,
                            node->line_number,
                            node->column_number
                    );
                }
            case Lexing::AND:
            case Lexing::OR:
                if (same_types && leftVisited.type->name == BOOL_TP) {
                    return bool_result;
                }else {
                    throw Exceptions::TypeException(
                            "(SemAn) " LOG_ONLY_BOOL_EXCP,
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
                "(SemAn) " UNA_ONLY_NUM_EXCP,
                node->line_number,
                node->column_number
        );
    }

    NodeResult SemanticAnalyzer::visit_TernaryOp(const std::shared_ptr<Parsing::TernaryOpNode>& node) {
        auto val_cond = visit(node->cond);

        // The condition of the ternary operator has to be boolean
        if (!val_cond.type || val_cond.type->name != BOOL_TP) {
            throw Exceptions::TypeException(
                    "(SemAn) " COND_TERN_MUST_BOOL_EXCP,
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
                    "(SemAn) " BRANCHES_MUST_RETURN_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        if (true_result.type != false_result.type) {
            // Error! Both branches must return the same value
            // TODO: Add type coersion
            throw Exceptions::TypeException(
                "(SemAn) " BOTH_BRANCH_SAME_TYPE_EXCP,
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
                    "(SemAn) " COND_IF_MUST_BOOL_EXCP,
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
                    "(SemAn) " COND_IF_MUST_BOOL_EXCP,
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        visit(node->incr);

        visit(node->body);

        currentScope = forScope.getParent();

        return {};
    }

    NodeResult SemanticAnalyzer::visit_ForEach(const std::shared_ptr<Parsing::ForEachNode>& node) {
        auto forScope = Interpreting::SymbolTable("foreach:loop", {}, currentScope);
        currentScope = &forScope;

        auto lst_value = visit(node->lst);

        if (!lst_value.type) {
            throw Exceptions::ValueException(
                "(SemAn) " NOTHING_TO_ITERATE_EXCP,
                node->line_number,
                node->column_number
            );
        }

        if (lst_value.type->kind == Interpreting::SymbolType::ListType) {
            std::shared_ptr<Node> iterator_decl;
            auto empty_initial = std::make_shared<NoOpNode>();
            auto element_tp = Lexing::Token(Lexing::TokenType::ID, lst_value.type->tp->name);
            if (lst_value.type->tp && lst_value.type->tp->kind == Interpreting::SymbolType::ListType) {
                iterator_decl = std::make_shared<ListDeclarationNode>(
                        std::move(element_tp),
                        node->var,
                        std::move(empty_initial)
                );
            } else {
                iterator_decl = std::make_shared<VarDeclarationNode>(std::move(element_tp), node->var, std::move(empty_initial));
            }

            visit(iterator_decl);

            visit(node->body);
        } else if (lst_value.type->name == STRING_TP) {
            auto iterator_decl = std::make_shared<VarDeclarationNode>(
                    Lexing::Token(Lexing::TokenType::ID, STRING_TP),
                    node->var,
                    std::make_shared<NoOpNode>()
            );
            visit(iterator_decl);

            visit(node->body);

        } else {
            throw Exceptions::ValueException(
                    "(SemAn) " FOREACH_ONLY_LIST_STR_EXCP,
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
                    "(SemAn) " VAL_RANGE_NUM_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        if (node->second && node->second->kind() != NodeType::NoOp) {

            auto second_visited = visit(node->second);
            if (!second_visited.type || !(second_visited.type->name == DOUBLE_TP || second_visited.type->name == INT_TP)) {
                throw Exceptions::ValueException(
                        "(SemAn) " VAL_RANGE_NUM_EXCP,
                        node->line_number,
                        node->column_number
                );
            }
        }

        bool use_iterator = node->var.tp != Lexing::NOTHING;

        if (use_iterator) {
            std::shared_ptr<Node> iterator_decl = std::make_shared<VarDeclarationNode>(
                    Lexing::Token(Lexing::TokenType::ID, INT_TP),
                    node->var,
                    std::make_shared<NoOpNode>()
            );
            visit(iterator_decl);
        }

        visit(node->body);

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
                    "(SemAn) " COND_WHILE_MUST_BOOL_EXCP,
                    node->cond->line_number,
                    node->cond->column_number
            );
        }

        visit(node->body);

        currentScope = whileScope.getParent();

        return {};
    }

    NodeResult SemanticAnalyzer::visit_Loop(const std::shared_ptr<Parsing::LoopNode>& node) {
        visit(node->body);
        return {};
    }


    NodeResult SemanticAnalyzer::visit_Index(const std::shared_ptr<Parsing::IndexNode>& node) {
        auto visited_val = visit(node->val);

        if (!visited_val.type) {
            // Error! Using index operator where there's no value to index.
            throw Exceptions::ValueException(
                "(SemAn) " NO_VALUE_TO_INDEX_EXCP,
                node->line_number,
                node->column_number
            );
        }

        if (visited_val.type->name == STRING_TP) {
            auto visited_indx = visit(node->expr);
            if (visited_indx.type->name != INT_TP) {
                throw Exceptions::TypeException(
                    "(SemAn) " STR_ONLY_INDX_NUM_EXCP,
                    node->line_number,
                    node->column_number
                );
            }
        } else if (visited_val.type->kind == Interpreting::SymbolType::ListType){
            auto visited_indx = visit(node->expr);
            if (visited_indx.type->name != INT_TP) {
                throw Exceptions::TypeException(
                    "(SemAn) " LST_ONLY_INDX_NUM_EXCP,
                    node->line_number,
                    node->column_number
                );
            }
        } else {
            throw Exceptions::ValueException(
                    "(SemAn) " INDX_ONLY_LST_STR_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        return {visited_val.type->tp, visited_val.is_constant};
    }

    NodeResult SemanticAnalyzer::visit_Block(const std::shared_ptr<Parsing::BlockNode>& node) {
        auto blockScope = Interpreting::SymbolTable("block_scope", {}, currentScope);
        currentScope = &blockScope;

        for (const auto& statement : node->statements) {
            visit(statement);
        }

        currentScope = blockScope.getParent();
        return {};
    }

    NodeResult SemanticAnalyzer::visit_VarDeclaration(const std::shared_ptr<Parsing::VarDeclarationNode>& node) {
        if (currentScope->symbolExists(node->name.value)) {
            throw Exceptions::NameException(
                    "(SemAn) " VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        auto type_ = currentScope->findSymbol(node->var_type.value);

        if (type_ == nullptr) {
            throw Exceptions::NameException(
                    "(SemAn) " UNKWN_TYPE_EXCP + node->var_type.value + "'.",
                    node->line_number,
                    node->column_number
            );
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
                    "(SemAn) " VAR_INIT_MUST_BE_VALID_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            if (!counts_as(newValue.type, type_)) {
                throw Exceptions::TypeException(
                        "(SemAn) " INVALID_DECL_TYPE_EXCP + type_->name + WITH_VAL_OF_TYPE_EXCP + newValue.type->name,
                        node->line_number,
                        node->column_number
                );
            }

            if (type_->name == ANY_TP) {
                newVar.tp = newValue.type;
                node->var_type = Lexing::Token(Lexing::TokenType::ID, newValue.type->name);
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
                    "(SemAn) " VAR_CALLED_EXCP + node->name.value + ALR_EXISTS_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        auto base_type = currentScope->findSymbol(node->var_type.value);
        if (!(base_type && base_type->isType)) {
            throw Exceptions::TypeException(
                    "(SemAn) " INVALID_TYPE_EXCP + node->var_type.value + "'.",
                    node->line_number,
                    node->column_number
            );
        }

        Interpreting::Symbol* list_type = inter.globalTable.addListType(base_type);
        bool was_init = false;
        bool is_constant = true;
        bool has_side_effects = false;

        if (node->initial && node->initial->kind() != NodeType::NoOp) {
            auto newValue = visit(node->initial);

            if (!newValue.type) {
                throw Exceptions::ValueException(
                    "(SemAn) " LST_INIT_MUST_BE_VALID_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            if (newValue.type->kind != Interpreting::SymbolType::ListType) {
                throw Exceptions::TypeException(
                    "(SemAn) " INVALID_LIST_INIT_NOT_LIST_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            if (!counts_as(newValue.type->tp, list_type->tp)) {
                throw Exceptions::TypeException(
                    "(SemAn) " INVALID_LST_DECL_TYPE_EXCP + list_type->tp->name + WITH_LST_VAL_OF_TYPE_EXCP + newValue.type->tp->name,
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

        auto element_template_name = "__" + list_type->name + "_list_element";

        if (!currentScope->symbolExists(element_template_name)) {
            auto list_el = currentScope->addSymbol({
                    list_type->tp,
                    element_template_name
            });

            list_el->is_initialized = was_init;
            list_el->content_is_constant = is_constant;
            list_el->content_has_side_effects = has_side_effects;
        }

        return {};
    }

    NodeResult SemanticAnalyzer::visit_Variable(const std::shared_ptr<Parsing::VariableNode>& node) {
        auto found = currentScope->findSymbol(node->token.value);

        if (found != nullptr) {
                // Check if initialized!
            if (found->is_initialized) {
                return {
                    found->tp,
                    found->content_is_constant,
                    found->content_has_side_effects
                };
            } else {
                throw Exceptions::ValueException(
                    "(SemAn) " USI_VAR_NOT_INIT_EXCP,
                    node->line_number,
                    node->column_number
                );
            }
        } else {
            throw Exceptions::NameException(
                "(SemAn) " VAR_CALLED_EXCP + node->token.value + NOT_DEFINED_EXCP,
                node->line_number,
                node->column_number
            );
        }
    }

    NodeResult SemanticAnalyzer::visit_Assignment(const std::shared_ptr<Parsing::AssignmentNode>& node) {
        auto varSym = getSymbolFromNode(node->expr);
        auto newValue = visit(node->val);

        if (varSym) {
            if (!varSym->is_initialized) {
                if (varSym->tp->name == ANY_TP) {
                    varSym->tp = newValue.type;
                }

                varSym->is_initialized = true;
            }

            if (!counts_as(newValue.type, varSym->tp)){
                throw Exceptions::TypeException(
                        "(SemAn) " INVALID_ASS_TYPE_EXCP + varSym->tp->name + WITH_VAL_OF_TYPE_EXCP + newValue.type->name,
                        node->line_number,
                        node->column_number
                );
            }

            varSym->content_is_constant = newValue.is_constant;
            varSym->content_has_side_effects = newValue.has_side_effects;
        } else {
            throw Exceptions::NameException(
                    "(SemAn) " ASS_TO_UNKWN_VAR_EXCP,
                    node->line_number,
                    node->column_number
            );
        }
        return {};
    }

    NodeResult SemanticAnalyzer::visit_ListExpression(const std::shared_ptr<Parsing::ListExpressionNode>& node) {
        NodeResult result{};
        bool is_any = false;

        for (const auto& el : node->elements) {
            auto el_result = visit(el);
            if (!el_result.type) {
                throw Exceptions::TypeException(
                    "(SemAn) " LST_EL_NO_VALUE_EXCP,
                    node->line_number,
                    node->column_number
                );
            }

            if (!result.type) {
                result.type = inter.globalTable.addListType(el_result.type);
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

    NodeResult SemanticAnalyzer::from_repl(const std::shared_ptr<Parsing::Node> & node) {
        auto temp_scope = currentScope;
        currentScope = &replScope;

        auto result = visit(node);

        currentScope = temp_scope;
        return result;
    }
}