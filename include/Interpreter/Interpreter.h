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

        std::vector<Value> constructorParams;

        SymbolTable globalTable;
        SymbolTable* currentScope;
        SymbolTable replScope;

        Value* null;
        Symbol* any_type;

        bool breaking = false;
        bool continuing = false;
        Value* returning;
        int callDepth = 0;

        unsigned int current_line;
        unsigned int current_col;

        std::map<std::string, NativeFunction> native_functions;

        Value* create_literal(std::string val, const std::string& kind);

        Value* visit_Double(Lexing::Token t);
        Value* visit_Int(Lexing::Token t);
        Value* visit_Bool(Lexing::Token t);
        Value* visit_Str(Lexing::Token t);

        Value* visit_Block(std::vector<Parsing::AST> vector);

        Value* visit_BinOp(Lexing::Token token, Parsing::AST &ast, Parsing::AST &ast1);
        Value* visit_UnaryOp(Lexing::Token token, Parsing::AST &ast);

        Value* visit_TernaryOp(Parsing::AST cond, Parsing::AST trueb, Parsing::AST falseb);


        Value* visit_If(Parsing::AST cond, Parsing::AST trueb, Parsing::AST falseb);
        Value* visit_For(Parsing::AST ini, Parsing::AST cond, Parsing::AST incr, Parsing::AST body);
        Value* visit_While(const Parsing::AST& cond, Parsing::AST body);
        Value* visit_Loop(Parsing::AST body);

        Value* visit_VarDeclaration(const Lexing::Token& var_type, Lexing::Token name, Parsing::AST initial);
        Value* visit_ListDeclaration(const Lexing::Token& var_type, Lexing::Token name, Parsing::AST initial);
        Value* visit_Assignment(Parsing::AST expr, Parsing::AST val);
        Value* visit_Variable(Lexing::Token token);

        Value* visit_Index(Parsing::AST val, Parsing::AST expr);

        Value* visit_ListExpression(std::vector<Parsing::AST> elements);

        Value* visit_FuncExpression(std::vector<Parsing::AST> params, const Lexing::Token& retType, Parsing::AST body);
        Value* visit_FuncDecl(const Lexing::Token& name, std::vector<Parsing::AST> params, Lexing::Token retType, Parsing::AST body);
        Value* visit_FuncCall(Parsing::AST expr, Lexing::Token fname, std::vector<Parsing::AST> args);
        Value* visit_FuncBody(std::vector<Parsing::AST> statements);
        Value* visit_Return(Parsing::AST val);

        std::vector<std::pair<Symbol, bool>> getParamTypes(const std::vector<Parsing::AST>&);

        Symbol *getMemberVarSymbol(const Parsing::AST& mem);

    public:
        void interpret(std::string);
        Value* eval(std::string);
        Value* visit(Parsing::AST node);
        explicit Interpreter(Parsing::Parser p=Parsing::Parser());
        std::string value_to_string(Value);
        int add_native_function(const std::string& name, NativeFunction callback);

        Value* get_null() { return null; }
    };
}
#endif //ODO_PORT_INTERPRETER_H
