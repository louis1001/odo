//
// Created by Luis Gonzalez on 3/20/20.
//

#ifndef ODO_PORT_INTERPRETER_H
#define ODO_PORT_INTERPRETER_H
#include "Parser/parser.h"
#include "Parser/AST.h"
#include "value.h"
#include "symbol.h"

#include <functional>
#include <vector>

#define MAX_CALL_DEPTH 1000
namespace Odo::Interpreting {
    typedef std::function<Value*(std::vector<Value*>)> NativeFunction;
    class Interpreter {
        Parsing::Parser parser;
        ValueTable valueTable;

        std::vector<Value *> constructorParams;

        SymbolTable globalTable;
        SymbolTable* currentScope;
        SymbolTable replScope;

        Value* null;
        Symbol* any_type;

        bool breaking = false;
        bool continuing = false;
        Value* returning;
        int callDepth = 0;

        unsigned int current_line{0};
        unsigned int current_col{0};

        std::map<std::string, NativeFunction> native_functions;

        Value* create_literal_from_string(std::string val, const std::string& kind);
        Value* create_literal_from_any(std::any val, const std::string& kind);
        Value* create_literal(std::string val);
        Value* create_literal(int val);
        Value* create_literal(double val);
        Value* create_literal(bool val);

        Value* visit_Double(const Lexing::Token& t);
        Value* visit_Int(const Lexing::Token& t);
        Value* visit_Bool(const Lexing::Token& t);
        Value* visit_Str(const Lexing::Token& t);

        Value* visit_Block(const std::vector<Parsing::AST>& vector);

        Value* visit_BinOp(const Lexing::Token& token, Parsing::AST &ast, Parsing::AST &ast1);
        Value* visit_UnaryOp(const Lexing::Token& token, Parsing::AST &ast);

        Value* visit_TernaryOp(const Parsing::AST& cond, Parsing::AST trueb, Parsing::AST falseb);


        Value* visit_If(const Parsing::AST& cond, Parsing::AST trueb, Parsing::AST falseb);
        Value* visit_For(Parsing::AST ini, const Parsing::AST& cond, const Parsing::AST& incr, const Parsing::AST& body);
        Value* visit_While(const Parsing::AST& cond, const Parsing::AST& body);
        Value* visit_Loop(const Parsing::AST& body);

        Value* visit_VarDeclaration(const Lexing::Token& var_type, const Lexing::Token& name, const Parsing::AST& initial);
        Value* visit_ListDeclaration(const Lexing::Token& var_type, const Lexing::Token& name, const Parsing::AST& initial);
        Value* visit_Assignment(Parsing::AST expr, Parsing::AST val);
        Value* visit_Variable(const Lexing::Token& token);

        Value* visit_Index(Parsing::AST val, const Parsing::AST& expr);

        Value* visit_ListExpression(const std::vector<Parsing::AST>& elements);

        Value* visit_Module(const Lexing::Token& name, const std::vector<Parsing::AST>& statements);
        Value *visit_Import(const Lexing::Token& path, const Lexing::Token& name);

        Value* visit_FuncExpression(const std::vector<Parsing::AST>& params, const Lexing::Token& retType, Parsing::AST body);
        Value* visit_FuncDecl(const Lexing::Token& name, const std::vector<Parsing::AST>& params, const Lexing::Token& retType, Parsing::AST body);
        Value* visit_FuncCall(Parsing::AST expr, const Lexing::Token& fname, std::vector<Parsing::AST> args);
        Value* visit_FuncBody(const std::vector<Parsing::AST>& statements);
        Value* visit_Return(Parsing::AST val);

        Value* visit_Class(const Lexing::Token& name, const Lexing::Token& ty, Parsing::AST body);
        Value* visit_ClassBody(std::vector<Parsing::AST> statements);

        Value* visit_ConstructorDecl(const std::vector<Parsing::AST>& params, Parsing::AST body);

        Value* visit_ConstructorCall(const Lexing::Token& t);

        Value* visit_ClassInitializer(const Lexing::Token& name, const std::vector<Parsing::AST>& params);

        Value* visit_InstanceBody(const std::vector<Parsing::AST>& statements);

        Value* visit_MemberVar(const Parsing::AST&, const Lexing::Token&);
        Value* visit_StaticVar(const Parsing::AST&, const Lexing::Token&);

        Value *interpret_as_module(const std::string &path, const Lexing::Token& name);

        std::vector<std::pair<Symbol, bool>> getParamTypes(const std::vector<Parsing::AST>&);

        Symbol *getMemberVarSymbol(Parsing::AST mem);

    public:
        void interpret(std::string);
        Value* eval(std::string);
        Value* visit(Parsing::AST node);
        explicit Interpreter(Parsing::Parser p=Parsing::Parser());
        int add_native_function(const std::string& name, NativeFunction callback);

        Value* get_null() { return null; }
    };
}
#endif //ODO_PORT_INTERPRETER_H
