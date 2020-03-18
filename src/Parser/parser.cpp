//
// Created by Luis Gonzalez on 3/17/20.
//

#include <utility>
#include <vector>
#include <algorithm>

#include "Parser/parser.h"

Parser::Parser(Lexer lexer) : lexer(std::move(lexer)) {}

void Parser::eat(TokenType tp) {
    if (current_token.tp == tp) {
        current_token = lexer.getNextToken();
    } else {
        // TODO: Handle exception
        throw 1;
    }
}

void Parser::set_text(std::string t) {
    lexer.reset();
    lexer.text = std::move(t);

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
    
    return AST { NoOp };
}

AST Parser::program() {
    auto result = block();

    if (current_token.tp != EOFT) {
        // TODO: Handle exceptions
        throw 1;
    }

    return result;
}

AST Parser::block() {
    return { Block, .lst_AST=statement_list() };
}

AST Parser::func_body(){
    return { FuncBody, .lst_AST=statement_list() };
}

AST Parser::class_body(){
    return { ClassBody, .lst_AST=statement_list() };
}

std::vector<AST> Parser::statement_list() {
    auto result = std::vector<AST>();

    ignore_nl();

    if(current_token.tp == RCUR) {
        return result;
    }

    while (current_token.tp != EOFT && current_token.tp == RCUR) {
        result.push_back(statement());
    }

    return result;
}

AST Parser::statement(bool with_term) {
    ignore_nl();

    switch (current_token.tp) {
        case LCUR:
        {
            eat(LCUR);
            auto content = block();
            eat(RCUR);
            return content;
        }
        case EOFT:
        case RCUR:
        case RPAR:
            return AST { NoOp };
        case STATIC:
            return AST {StaticStatement, .nodes={
                    {
                        "statement",
                        statement(false)
                    }
                }
            };
            return { NoOp };
        case DEBUG:
            return AST{Debug};
        default:
            break;
    }
    return AST { NoOp };
}

