//
// Created by Luis Gonzalez on 12/19/20.
//

#include "SemAnalyzer/SemanticAnalyzer.h"
#include "Exceptions/exception.h"
#include "Interpreter/Interpreter.h"

#define TEST_SEMANTICS

namespace Odo::Semantics {
    using namespace Parsing;
    // This should initialize the symbol tables and basic types, like the Interpreter.
    SemanticAnalyzer::SemanticAnalyzer(Interpreting::Interpreter& inter_): inter(inter_) {
        currentScope = &inter.globalTable;
        replScope = { "repl-analyzer", {}, &inter.globalTable };
    };

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
                // return visit_BinOp(Node::as<BinOpNode>(node));
                /* ToRemoveLater */ break;
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
                // return visit_ListDeclaration(Node::as<ListDeclarationNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::Variable:
                // return visit_Variable(Node::as<VariableNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::Assignment:
                // return visit_Assignment(Node::as<AssignmentNode>(node));
                /* ToRemoveLater */ break;

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
                // return visit_Enum(Node::as<EnumNode>(node));
                /* ToRemoveLater */ break;

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
//                        current_line,
//                        current_col
//                );
            case NodeType::MemberVar:
                // return visit_MemberVar(Node::as<MemberVarNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::StaticVar:
                // return visit_StaticVar(Node::as<StaticVarNode>(node));
                /* ToRemoveLater */ break;

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

        Interpreting::Symbol newVar;
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

            if (type_->name == ANY_TP) {
                type_ = newValue.type;
            } else if (type_ != newValue.type) {
                // TODO: Add Type Coersion
                // TODO: Also Check For Inheritance
                throw Exceptions::TypeException(
                    "(SemAn) " INVALID_DECL_TYPE_EXCP + type_->name + WITH_VAL_OF_TYPE_EXCP + newValue.type->name,
                    node->line_number,
                    node->column_number
                );
            }

            newVar = Interpreting::Symbol{
                type_,
                node->name.value
            };
        } else {
            newVar = {
                type_,
                node->name.value
            };

        }

        currentScope->addSymbol(newVar);
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

    NodeResult SemanticAnalyzer::from_repl(const std::shared_ptr<Parsing::Node> & node) {
        auto temp_scope = currentScope;
        currentScope = &replScope;

        auto result = visit(node);

        currentScope = temp_scope;
        return result;
    }
}