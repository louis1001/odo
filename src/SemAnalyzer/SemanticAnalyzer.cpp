//
// Created by Luis Gonzalez on 12/19/20.
//

#include "SemAnalyzer/SemanticAnalyzer.h"
#include "Exceptions/exception.h"

#define TEST_SEMANTICS

namespace Odo::Semantics {
    using namespace Parsing;
    // This should initialize the symbol tables and basic types, like the Interpreter.
    SemanticAnalyzer::SemanticAnalyzer(Interpreting::Interpreter& inter_): inter(inter_) {};

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
                // return visit_UnaryOp(Node::as<UnaryOpNode>(node));
                /* ToRemoveLater */ break;
            case NodeType::NoOp:
                // return null;
                /* ToRemoveLater */ break;

            case NodeType::Index:
                // return visit_Index(Node::as<IndexNode>(node));
                /* ToRemoveLater */ break;

                // Control Flow
            case NodeType::TernaryOp:
                // return visit_TernaryOp(Node::as<TernaryOpNode>(node));
                /* ToRemoveLater */ break;
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
                // return visit_ListExpression(Node::as<ListExpressionNode>(node));
                /* ToRemoveLater */ break;

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
                // return null;
                break;
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

    NodeResult SemanticAnalyzer::visit_Block(const std::shared_ptr<Parsing::BlockNode>& node) {
        for (const auto& statement : node->statements) {
            visit(statement);
        }
        return {};
    }
}