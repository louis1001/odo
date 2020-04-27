//
// Created by Luis Gonzalez on 3/17/20.
//

#ifndef ODO_PORT_PARSER_H
#define ODO_PORT_PARSER_H

#include "lexer.hpp"
#include "token.hpp"
#include "AST.h"

#include <vector>
namespace Odo::Parsing {
    class Parser {
        Lexing::Lexer lexer;
        Lexing::Token current_token = Lexing::Token(Lexing::EOFT, "");

        AST add_dbg_info(AST);

        void eat(Lexing::TokenType tp);

        AST block();

        AST func_body();
        AST class_body();

        std::vector<AST> statement_list();

        void statement_terminator();

        void ignore_nl();

        AST statement(bool with_term = true);

        AST module_statement();

        AST classstatement();
        AST newInitializer();
        AST constructor();

        AST loopstatement();
        AST whilestatement();
        AST forstatement();

        AST ifstatement();

        AST function();
        std::vector<AST> parameters();

        AST declaration(const Lexing::Token&);

        AST ternary_op();

        AST or_comparison();
        AST and_comparison();
        AST comparison();

        AST expression();
        AST term();
        AST prod();
        AST postfix();

        AST funcexpression();

        AST factor();
    public:

        Parser();
        AST program();
        std::vector<AST> program_content();
        void set_text(std::string t);
    };
}
#endif //ODO_PORT_PARSER_H
