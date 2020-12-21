//
// Created by Luis Gonzalez on 12/19/20.
//

#ifndef ODO_SEMANTICANALYZER_H
#define ODO_SEMANTICANALYZER_H

#include "NodeResult.h"

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

namespace Odo::Interpreting {
    class Interpreter;
}

namespace Odo::Semantics {
    class SemanticAnalyzer {
        Interpreting::SymbolTable replScope;
        Interpreting::SymbolTable* currentScope;

        Interpreting::Interpreter& inter;

        NodeResult visit_Double(const std::shared_ptr<Parsing::DoubleNode>&);
        NodeResult visit_Int(const std::shared_ptr<Parsing::IntNode>&);
        NodeResult visit_Str(const std::shared_ptr<Parsing::StrNode>&);
        NodeResult visit_Bool(const std::shared_ptr<Parsing::BoolNode>&);

        NodeResult visit_UnaryOp(const std::shared_ptr<Parsing::UnaryOpNode>&);

        NodeResult visit_TernaryOp(const std::shared_ptr<Parsing::TernaryOpNode>&);
        NodeResult visit_If(const std::shared_ptr<Parsing::IfNode>&);
        NodeResult visit_For(const std::shared_ptr<Parsing::ForNode>&);
        NodeResult visit_ForEach(const std::shared_ptr<Parsing::ForEachNode>&);
        NodeResult visit_FoRange(const std::shared_ptr<Parsing::FoRangeNode>&);
        NodeResult visit_While(const std::shared_ptr<Parsing::WhileNode>&);
        NodeResult visit_Loop(const std::shared_ptr<Parsing::LoopNode>&);

        NodeResult visit_Index(const std::shared_ptr<Parsing::IndexNode>&);

        NodeResult visit_Block(const std::shared_ptr<Parsing::BlockNode>&);

        NodeResult visit_VarDeclaration(const std::shared_ptr<Parsing::VarDeclarationNode>&);

        NodeResult visit_ListExpression(const std::shared_ptr<Parsing::ListExpressionNode>&);

    public:
        explicit SemanticAnalyzer(Interpreting::Interpreter&);

        NodeResult visit(const std::shared_ptr<Parsing::Node>&);
        NodeResult from_repl(const std::shared_ptr<Parsing::Node>&);
    };
}

#endif //ODO_SEMANTICANALYZER_H
