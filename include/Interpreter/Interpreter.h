//
// Created by Luis Gonzalez on 3/20/20.
//

#ifndef ODO_PORT_INTERPRETER_H
#define ODO_PORT_INTERPRETER_H
#include "Parser/parser.h"
#include "Parser/AST/Node.h"
#include "Parser/AST/Forward.h"
#include "value.h"
#include "symbol.h"
#include "frame.h"

#include <functional>
#include <vector>

#define MAX_CALL_DEPTH 800
namespace Odo::Interpreting {
    typedef std::function<std::shared_ptr<Value>(std::vector<std::shared_ptr<Value>>)> NativeFunction;
    class Interpreter {
        Parsing::Parser parser;

        std::vector<std::shared_ptr<Value>> constructorParams;

        SymbolTable globalTable;
        SymbolTable* currentScope;
        SymbolTable replScope;

        std::shared_ptr<Value> null;
        Symbol* any_type();

        bool breaking = false;
        bool continuing = false;
        std::shared_ptr<Value> returning;
        std::shared_ptr<Value> returning_native;
        std::vector<Frame> call_stack;

        unsigned int current_line{0};
        unsigned int current_col{0};

        std::map<std::string, NativeFunction> native_functions;

        std::pair<std::shared_ptr<Value>, std::shared_ptr<Value>>
        coerce_type(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs);

        std::shared_ptr<Value> create_literal_from_string(std::string val, const std::string& kind);
        std::shared_ptr<Value> create_literal_from_any(const std::any& val, const std::string& kind);
        std::shared_ptr<Value> create_literal(std::string val);
        std::shared_ptr<Value> create_literal(int val);
        std::shared_ptr<Value> create_literal(double val);
        std::shared_ptr<Value> create_literal(bool val);

        std::shared_ptr<Value> visit_Double(const std::shared_ptr<Parsing::DoubleNode>&);
        std::shared_ptr<Value> visit_Int(const std::shared_ptr<Parsing::IntNode>&);
        std::shared_ptr<Value> visit_Bool(const std::shared_ptr<Parsing::BoolNode>&);
        std::shared_ptr<Value> visit_Str(const std::shared_ptr<Parsing::StrNode>&);

        std::shared_ptr<Value> visit_Block(const std::shared_ptr<Parsing::BlockNode>&);

        std::shared_ptr<Value> visit_BinOp(const std::shared_ptr<Parsing::BinOpNode>&);
        std::shared_ptr<Value> visit_UnaryOp(const std::shared_ptr<Parsing::UnaryOpNode>&);

        std::shared_ptr<Value> visit_TernaryOp(const std::shared_ptr<Parsing::TernaryOpNode>&);


        std::shared_ptr<Value> visit_If(const std::shared_ptr<Parsing::IfNode>&);
        std::shared_ptr<Value> visit_For(const std::shared_ptr<Parsing::ForNode>&);
        std::shared_ptr<Value> visit_ForEach(const std::shared_ptr<Parsing::ForEachNode>&);
        std::shared_ptr<Value> visit_FoRange(const std::shared_ptr<Parsing::FoRangeNode>&);
        std::shared_ptr<Value> visit_While(const std::shared_ptr<Parsing::WhileNode>&);
        std::shared_ptr<Value> visit_Loop(const std::shared_ptr<Parsing::LoopNode>&);

        std::shared_ptr<Value> visit_VarDeclaration(const std::shared_ptr<Parsing::VarDeclarationNode>&);
        std::shared_ptr<Value> visit_ListDeclaration(const std::shared_ptr<Parsing::ListDeclarationNode>&);
        std::shared_ptr<Value> visit_Assignment(const std::shared_ptr<Parsing::AssignmentNode>&);
        std::shared_ptr<Value> visit_Variable(const std::shared_ptr<Parsing::VariableNode>&);

        std::shared_ptr<Value> visit_Index(const std::shared_ptr<Parsing::IndexNode>&);

        std::shared_ptr<Value> visit_ListExpression(const std::shared_ptr<Parsing::ListExpressionNode>&);

        std::shared_ptr<Value> visit_Module(const std::shared_ptr<Parsing::ModuleNode>&);
        std::shared_ptr<Value> visit_Import(const std::shared_ptr<Parsing::ImportNode>&);

        std::shared_ptr<Value> visit_FuncExpression(const std::shared_ptr<Parsing::FuncExpressionNode>&);
        std::shared_ptr<Value> visit_FuncDecl(const std::shared_ptr<Parsing::FuncDeclNode>&);
        std::shared_ptr<Value> visit_FuncCall(const std::shared_ptr<Parsing::FuncCallNode>&);
        std::shared_ptr<Value> visit_FuncBody(const std::shared_ptr<Parsing::FuncBodyNode>&);
        std::shared_ptr<Value> visit_Return(const std::shared_ptr<Parsing::ReturnNode>&);

        std::shared_ptr<Value> visit_Enum(const std::shared_ptr<Parsing::EnumNode>&);

        std::shared_ptr<Value> visit_Class(const std::shared_ptr<Parsing::ClassNode>&);
        std::shared_ptr<Value> visit_ClassBody(const std::shared_ptr<Parsing::ClassBodyNode>&);

        std::shared_ptr<Value> visit_ConstructorDecl(const std::shared_ptr<Parsing::ConstructorDeclNode>&);

        std::shared_ptr<Value> visit_ConstructorCall(const std::shared_ptr<Parsing::ConstructorCallNode>&);

        std::shared_ptr<Value> visit_ClassInitializer(const std::shared_ptr<Parsing::ClassInitializerNode>&);

        std::shared_ptr<Value> visit_InstanceBody(const std::shared_ptr<Parsing::InstanceBodyNode>&);

        std::shared_ptr<Value> visit_MemberVar(const std::shared_ptr<Parsing::MemberVarNode>&);
        std::shared_ptr<Value> visit_StaticVar(const std::shared_ptr<Parsing::StaticVarNode>&);

        std::shared_ptr<Value> interpret_as_module(const std::string &path, const Lexing::Token& name);

        std::vector<std::pair<Symbol, bool>> getParamTypes(const std::vector<std::shared_ptr<Parsing::Node>>&);

        Symbol *getMemberVarSymbol(const std::shared_ptr<Parsing::Node>& mem);

    public:
        void interpret(std::string);
        std::shared_ptr<Value> eval(std::string);
        std::shared_ptr<Value> visit(const std::shared_ptr<Parsing::Node>& node);
        explicit Interpreter(Parsing::Parser p=Parsing::Parser());
        int add_native_function(const std::string& name, NativeFunction callback);
        void set_repl_last(std::shared_ptr<Value> v);

        std::vector<Frame>& get_call_stack() { return call_stack; };
        std::shared_ptr<Value> get_null() { return null; }
    };
}
#endif //ODO_PORT_INTERPRETER_H
