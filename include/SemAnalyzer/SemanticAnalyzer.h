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

#define ADD_VISITOR(X) NodeResult visit_ ## X(const std::shared_ptr<Parsing::X ## Node>&)

namespace Odo::Semantics {
    class SemanticAnalyzer {
        Interpreting::SymbolTable replScope;
        Interpreting::SymbolTable* currentScope;

        Interpreting::Interpreter& inter;

        std::map<Interpreting::Symbol*, Interpreting::SymbolTable> semantic_contexts;
        std::vector<Interpreting::Symbol*> to_clean;

        Interpreting::SymbolTable* add_semantic_context(Interpreting::Symbol*, std::string);
        Interpreting::SymbolTable* get_semantic_context(Interpreting::Symbol*);

        void clean_contexts();

        ADD_VISITOR(Double);
        ADD_VISITOR(Int);
        ADD_VISITOR(Str);
        ADD_VISITOR(Bool);

        ADD_VISITOR(BinOp);
        ADD_VISITOR(UnaryOp);

        ADD_VISITOR(TernaryOp);
        ADD_VISITOR(If);
        ADD_VISITOR(For);
        ADD_VISITOR(ForEach);
        ADD_VISITOR(FoRange);
        ADD_VISITOR(While);
        ADD_VISITOR(Loop);

        ADD_VISITOR(Index);

        ADD_VISITOR(Block);

        ADD_VISITOR(VarDeclaration);
        ADD_VISITOR(ListDeclaration);
        ADD_VISITOR(Variable);
        ADD_VISITOR(Assignment);

        ADD_VISITOR(ListExpression);

        ADD_VISITOR(Enum);

        ADD_VISITOR(StaticVar);

        ADD_VISITOR(Module);

        bool counts_as(Interpreting::Symbol* type1, Interpreting::Symbol* type2);

        Interpreting::Symbol *getSymbolFromNode(const std::shared_ptr<Parsing::Node>& mem);

    public:
        explicit SemanticAnalyzer(Interpreting::Interpreter&);

        NodeResult visit(const std::shared_ptr<Parsing::Node>&);
        NodeResult from_repl(const std::shared_ptr<Parsing::Node>&);
    };
}

#endif //ODO_SEMANTICANALYZER_H
