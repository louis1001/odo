//
// Created by Luis Gonzalez on 3/17/20.
//

#include <vector>
#include <algorithm>

#include "Parser/parser.h"

Parser::Parser(const Lexer &lexer) : lexer(lexer) {}

void Parser::eat(TokenType tp) {
    if (current_token.tp == tp) {
        current_token = lexer.getNextToken();
    } else {
        // TODO: Handle exeption
        throw 1;
    }
}

void Parser::set_text(std::string t) {
    lexer.reset();
    lexer.text = t;

    current_token = lexer.getNextToken();
}

void Parser::ignore_nl() {
    while (current_token.tp == NL) {
        eat(NL);
    }
}

void Parser::statement_terminator() {
    std::vector<TokenType> block_terminator = {
        EOFT,
        RCUR,
        RPAR
    };

    if (current_token.tp == SEMI) {
        eat(SEMI);
        while (current_token.tp == NL) {
            eat(NL);
        }
    } else if (std::find(
            block_terminator.begin(),
            block_terminator.end(),
            current_token.tp
        ) == block_terminator.end()){
        do {
            eat(NL);
        } while (current_token.tp == NL);
    }
}

AST Parser::factor() {
    
    return AST();
}

