//
// Created by Luis Gonzalez on 3/17/20.
//

#ifndef ODO_PORT_PARSER_H
#define ODO_PORT_PARSER_H

#include "lexer.hpp"
#include "token.hpp"
#include "AST.h"

class Parser {
    Lexer lexer;
    Token current_token = Token(EOFT, "");

    void eat(TokenType tp);

    AST program();

    AST block();

    AST func_body();
    AST class_body();

    AST statement_list();

    void statement_terminator();

    void ignore_nl();

    AST statement(bool with_term);

    AST classstatement();
    AST newInitializer();
    AST constructor();

    AST loopstatement();
    AST whilestatement();
    AST forstatement();

    AST ifstatement();

    AST function();
    AST parameters();

    AST declaration();

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
    explicit Parser(const Lexer &lexer);
    void set_text(std::string t);
};

#endif //ODO_PORT_PARSER_H
