//
// Created by Luis Gonzalez on 3/17/20.
//

#ifndef ODO_PORT_PARSER_H
#define ODO_PORT_PARSER_H

#include "lexer.hpp"
#include "token.hpp"
#include "AST/Node.h"

#include <vector>
namespace Odo::Parsing {
    class Parser {
        Lexing::Lexer lexer;
        Lexing::Token current_token = Lexing::Token(Lexing::EOFT, "");

        std::shared_ptr<Node> add_dbg_info(std::shared_ptr<Node>);
        unsigned int line() { return lexer.getCurrentLine(); }
        unsigned int column() { return lexer.getCurrentCol(); }

        void eat(Lexing::TokenType tp);

        struct function_type {
            std::vector<std::pair<Lexing::Token, bool>> args;
            Lexing::Token retType;
        };

        std::shared_ptr<Node> block();

        std::shared_ptr<Node> func_body();
        std::shared_ptr<Node> class_body();

        std::vector<std::shared_ptr<Node>> statement_list();

        void statement_terminator();

        void ignore_nl();

        std::shared_ptr<Node> statement(bool with_term = true);

        std::shared_ptr<Node> module_statement();

        std::shared_ptr<Node> get_type();

        function_type get_arg_types();
        std::shared_ptr<Node> define_statement();

        std::shared_ptr<Node> enumstatement();

        std::shared_ptr<Node> classstatement();
        std::shared_ptr<Node> newInitializer();
        std::shared_ptr<Node> constructor();

        std::shared_ptr<Node> loopstatement();
        std::shared_ptr<Node> whilestatement();
        std::shared_ptr<Node> forstatement();
        std::shared_ptr<Node> foreachstatement();
        std::shared_ptr<Node> forangestatement();

        std::shared_ptr<Node> ifstatement();

        std::shared_ptr<Node> function();
        std::vector<std::shared_ptr<Node>> parameters();
        std::vector<std::shared_ptr<Node>> call_args();

        std::shared_ptr<Node> declaration(const Lexing::Token&);

        std::shared_ptr<Node> ternary_op();

        std::shared_ptr<Node> or_comparison();
        std::shared_ptr<Node> and_comparison();
        std::shared_ptr<Node> comparison();

        std::shared_ptr<Node> expression();
        std::shared_ptr<Node> term();
        std::shared_ptr<Node> prod();
        std::shared_ptr<Node> postfix();

        std::shared_ptr<Node> funcexpression();

        std::shared_ptr<Node> factor();
    public:

        Parser();
        std::shared_ptr<Node> program();
        std::vector<std::shared_ptr<Node>> program_content();
        void set_text(std::string t);
    };
}
#endif //ODO_PORT_PARSER_H
