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
                // return visit_If(Node::as<IfNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::For:
                // return visit_For(Node::as<ForNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::ForEach:
                // return visit_ForEach(Node::as<ForEachNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::FoRange:
                // return visit_FoRange(Node::as<FoRangeNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::While:
                // return visit_While(Node::as<WhileNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::Loop:
                // return visit_Loop(Node::as<LoopNode>(node));
                /* ToRemoveLater */ break;
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
                // return visit_VarDeclaration(Node::as<VarDeclarationNode>(node));
                /* ToRemoveLater */ break;
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
                UNA_ONLY_NUM_EXCP,
                node->line_number,
                node->column_number
        );
    }

    NodeResult SemanticAnalyzer::visit_TernaryOp(const std::shared_ptr<Parsing::TernaryOpNode>& node) {
        auto val_cond = visit(node->cond);

        // The condition of the ternary operator has to be boolean
        if (val_cond.type && val_cond.type->name != BOOL_TP) {
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

        if (true_result.type != false_result.type) {
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
        } else if (visited_val.type->kind == Interpreting::SymbolType::ListType){
            auto visited_indx = visit(node->expr);
            if (visited_indx.type->name != INT_TP) {
                throw Exceptions::TypeException(
                    LST_ONLY_INDX_NUM_EXCP,
                    node->line_number,
                    node->column_number
                );
            }
        } else {
            throw Exceptions::ValueException(
                    INDX_ONLY_LST_STR_EXCP,
                    node->line_number,
                    node->column_number
            );
        }

        return {visited_val.type->tp, visited_val.is_constant};
    }

    NodeResult SemanticAnalyzer::visit_Block(const std::shared_ptr<Parsing::BlockNode>& node) {
        for (const auto& statement : node->statements) {
            visit(statement);
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
                    LST_EL_NO_VALUE_EXCP,
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