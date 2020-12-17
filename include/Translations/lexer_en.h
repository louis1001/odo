//
// Created by admin on 16/12/2020.
//

#ifndef ODO_LEXER_EN_H
#define ODO_LEXER_EN_H

// Keyword tokens
#define TRUE_TK "true"
#define FALSE_TK "false"
#define IF_TK "if"
#define ELSE_TK "else"
#define FUNC_TK "func"
#define RETURN_TK "return"
#define FOR_TK "for"
#define FOREACH_TK "foreach"
#define FORANGE_TK "forange"
#define WHILE_TK "while"
#define LOOP_TK "loop"
#define BREAK_TK "break"
#define CONTINUE_TK "continue"
#define MODULE_TK "module"
#define IMPORT_TK "import"
#define AS_TK "as"
#define ENUM_TK "enum"
#define CLASS_TK "class"
#define NEW_TK "new"
#define STATIC_TK "static"
#define INIT_TK "init"
#define NULL_TK "null"
#define DEBUG_TK "debug"

// Exceptions
#define END_COMMENT_EXCP "Missing end of comment: */"
#define END_STRING_EXCP "Missing end of string: "
#define UNKWN_CHAR_EXCP "Unknown character '"

#endif //ODO_LEXER_EN_H
