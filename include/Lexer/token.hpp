#pragma once
#include <string>

enum TokenType {
    NULLT,           // empty value
    EOFT,            // empty token
    
    // Built in literals
    REAL,           // Floating Point number literal(e.g: 1.0, 2.0, 3.1415)
    INT,            // Integer number literal  (e.g: 1, 2, 100)
    BOOL,           // Boolean literal (e.g: true, false)
    STR,            // String of text literal (e.g: "Hello, world")

    ID,             // Identifier (e.g: i, var1)

    NL,             // Newline character (\n)
    COMMA,          // ,
    SEMI,           // ;
    COLON,          // :

    DOT,
    DCOLON,
    QUEST,          // ?

    ARROW,          // ->

    // Operators
    PLUS,           // +
    PLUSE,          // +=
    PLUSP,          // ++
    MINUS,          // -
    MINUSE,         // -=
    MINUSP,         // --

    MUL,            // *
    MULE,           // *=
    DIV,            // /
    DIVE,           // /=

    AND,            // &&
    OR,             // ||

    POW,            // ^
    POWE,           // ^=

    ASS,            // =

    EQU,            // ==
    NOT,            // !
    NEQ,            // !=
    LT,             // <
    GT,             // >
    LET,            // <=
    GET,            // >=

    // Delimiters
    LPAR,           // (
    RPAR,           // )
    LCUR,           // {
    RCUR,           // }
    LBRA,           // [
    RBRA,           // ]

    // Keywords
    // Represent their literal value
    IF,
    ELSE,
    FUNC,
    RET,
    FOR,
    WHILE,
    LOOP,
    BREAK,

    DEBUG,

    CLASS,
    INIT,
    NEW,
    STATIC,
};

struct Token {
    TokenType tp;
    std::string value;

    Token(TokenType tp_, std::string val_);
};
