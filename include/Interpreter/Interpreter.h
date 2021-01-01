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
#include "SemAnalyzer/SemanticAnalyzer.h"
#include "Modules/NativeModule.h"

#include <functional>
#include <vector>

#define INTER_VISITOR(X) std::shared_ptr<Value> visit_ ## X(const std::shared_ptr<Parsing::X ## Node>&)

#define MAX_CALL_DEPTH 800
namespace Odo::Interpreting {
    typedef std::function<std::shared_ptr<Value>(std::vector<std::shared_ptr<Value>>)> NativeFunction;
    class Interpreter {
        Parsing::Parser parser;

        std::shared_ptr<Semantics::SemanticAnalyzer> analyzer {nullptr};

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

        void add_function(const std::string&, const std::vector<std::pair<Symbol*, bool>>&, Symbol*, std::function<std::any(std::vector<std::any>)>);

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

        INTER_VISITOR(Double);
        INTER_VISITOR(Int);
        INTER_VISITOR(Bool);
        INTER_VISITOR(Str);

        INTER_VISITOR(Block);

        INTER_VISITOR(BinOp);
        INTER_VISITOR(UnaryOp);

        INTER_VISITOR(TernaryOp);


        INTER_VISITOR(If);
        INTER_VISITOR(For);
        INTER_VISITOR(ForEach);
        INTER_VISITOR(FoRange);
        INTER_VISITOR(While);
        INTER_VISITOR(Loop);

        INTER_VISITOR(VarDeclaration);
        INTER_VISITOR(ListDeclaration);
        INTER_VISITOR(Assignment);
        INTER_VISITOR(Variable);

        INTER_VISITOR(Index);

        INTER_VISITOR(ListExpression);

        INTER_VISITOR(Module);
        INTER_VISITOR(Import);

        INTER_VISITOR(Define);

        INTER_VISITOR(FuncExpression);
        INTER_VISITOR(FuncDecl);
        INTER_VISITOR(FuncCall);
        INTER_VISITOR(FuncBody);
        INTER_VISITOR(Return);

        INTER_VISITOR(Enum);

        INTER_VISITOR(Class);
        INTER_VISITOR(ClassBody);

        INTER_VISITOR(ConstructorDecl);

        INTER_VISITOR(ConstructorCall);

        INTER_VISITOR(ClassInitializer);

        INTER_VISITOR(InstanceBody);

        INTER_VISITOR(MemberVar);
        INTER_VISITOR(StaticVar);

        std::shared_ptr<Value> interpret_as_module(const std::string &path, const Lexing::Token& name);

        std::vector<std::pair<Symbol*, bool>> getParamTypes(const std::vector<std::shared_ptr<Parsing::Node>>&);

        Symbol *getSymbolFromNode(const std::shared_ptr<Parsing::Node>& mem);

        friend class Semantics::SemanticAnalyzer;
    public:
        void interpret(std::string);
        std::shared_ptr<Value> eval(std::string);
        std::shared_ptr<Value> visit(const std::shared_ptr<Parsing::Node>& node);
        explicit Interpreter(Parsing::Parser p=Parsing::Parser());
        int add_native_function(const std::string& name, NativeFunction callback);

        SymbolTable& get_global() { return globalTable; };

        std::shared_ptr<Semantics::SemanticAnalyzer> get_analyzer() { return analyzer; }

        void add_module(std::shared_ptr<Modules::NativeModule>);
        void set_repl_last(std::shared_ptr<Value> v);

        std::vector<Frame>& get_call_stack() { return call_stack; };
        std::shared_ptr<Value> get_null() { return null; }
    };
}
#endif //ODO_PORT_INTERPRETER_H
