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

#define INTER_VISITOR(X) value_t visit_ ## X(const std::shared_ptr<Parsing::X ## Node>&)

#define MAX_CALL_DEPTH 800
namespace Odo::Interpreting {
    typedef std::shared_ptr<Value> value_t;
    typedef std::function<value_t(std::vector<value_t>)> NativeFunction;
    class Interpreter {
        Parsing::Parser parser;

        std::shared_ptr<Semantics::SemanticAnalyzer> analyzer {nullptr};

        std::vector<value_t> constructorParams;

        SymbolTable globalTable;
        SymbolTable* currentScope;
        SymbolTable replScope;

        value_t null;
        Symbol* any_type();

        bool breaking = false;
        bool continuing = false;
        value_t returning;
        value_t returning_native;
        std::vector<Frame> call_stack;

        void add_function(const std::string&, const std::vector<std::pair<Symbol*, bool>>&, Symbol*, const std::function<std::any(std::vector<std::any>)>&);
        void add_function(const std::string&, const std::vector<std::pair<Symbol*, bool>>&, Symbol*, const std::function<value_t(
        std::vector<value_t>)>&);

        unsigned int current_line{0};
        unsigned int current_col{0};

        std::map<std::string, NativeFunction> native_functions;

        std::pair<value_t, value_t>
        coerce_type(const value_t& lhs, const value_t& rhs);

        Symbol* int_type;
        Symbol* double_type;
        Symbol* string_type;
        Symbol* bool_type;

        value_t create_literal_from_string(std::string val, const std::string& kind);
        value_t create_literal(std::string val);
        value_t create_literal(int val);
        value_t create_literal(double val);
        value_t create_literal(bool val);

        INTER_VISITOR(Double);
        INTER_VISITOR(Int);
        INTER_VISITOR(Bool);
        INTER_VISITOR(Str);

        INTER_VISITOR(Block);

        INTER_VISITOR(BinOp);

        value_t visit_BinOp_arit(const std::shared_ptr<Parsing::BinOpNode>& node);
        value_t visit_BinOp_equa(const std::shared_ptr<Parsing::BinOpNode>& node);
        value_t visit_BinOp_rela(const std::shared_ptr<Parsing::BinOpNode>& node);
        value_t visit_BinOp_bool(const std::shared_ptr<Parsing::BinOpNode>& node);

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

        value_t interpret_as_module(const std::string &path, const Lexing::Token& name);

        std::vector<std::pair<Symbol*, bool>> getParamTypes(const std::vector<std::shared_ptr<Parsing::Node>>&);

        Symbol *getSymbolFromNode(const std::shared_ptr<Parsing::Node>& mem);

        friend class Semantics::SemanticAnalyzer;
    public:
        void interpret(std::string);
        value_t eval(std::string);
        value_t visit(const std::shared_ptr<Parsing::Node>& node);
        explicit Interpreter(Parsing::Parser p=Parsing::Parser());
        int add_native_function(const std::string& name, NativeFunction callback);
        void add_function(const std::string &name, const std::function<void()> &callback);

        SymbolTable& get_global() { return globalTable; };

        std::shared_ptr<Semantics::SemanticAnalyzer> get_analyzer() { return analyzer; }

        void add_module(std::shared_ptr<Modules::NativeModule>);
        void set_repl_last(value_t v);

        std::vector<Frame>& get_call_stack() { return call_stack; };
        value_t get_null() { return null; }
    };
}
#endif //ODO_PORT_INTERPRETER_H
