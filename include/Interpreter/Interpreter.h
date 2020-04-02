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
typedef std::function<Value*(std::vector<Value*>)> NativeFunction;

class Interpreter {
    Parser parser;
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

    std::map<std::string, NativeFunction> native_functions;
    int add_native_function(const std::string& name, NativeFunction callback);

    Value* create_literal(std::string val, const std::string& kind);

    Value* visit_Double(Token t);
    Value* visit_Int(Token t);
    Value* visit_Bool(Token t);
    Value* visit_Str(Token t);

    Value* visit_Block(std::vector<AST> vector);

    Value* visit_BinOp(Token token, AST &ast, AST &ast1);
    Value* visit_UnaryOp(Token token, AST &ast);

    Value* visit_TernaryOp(AST cond, AST trueb, AST falseb);


    Value* visit_If(AST cond, AST trueb, AST falseb);
    Value* visit_For(AST ini, AST cond, AST incr, AST body);
    Value* visit_While(const AST& cond, AST body);
    Value* visit_Loop(AST body);

    Value* visit_VarDeclaration(const Token& var_type, Token name, AST initial);
    Value* visit_ListDeclaration(const Token& var_type, Token name, AST initial);
    Value* visit_Assignment(AST expr, AST val);
    Value* visit_Variable(Token token);

    Value* visit_Index(AST val, AST expr);

    Value* visit_ListExpression(std::vector<AST> elements);

    Value* visit_FuncCall(AST expr, Token fname, std::vector<AST> args);

    Symbol *getMemberVarSymbol(const AST& mem);

public:
    Value* visit(AST node);
    explicit Interpreter(Parser p=Parser());
    std::string value_to_string(Value);

};

#endif //ODO_PORT_INTERPRETER_H
