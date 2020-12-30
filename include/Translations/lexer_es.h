//
// Created by admin on 16/12/2020.
//

#ifndef ODO_LEXER_ES_H
#define ODO_LEXER_ES_H

// Keyword tokens
#define TRUE_TK "cierto"
#define FALSE_TK "falso"
#define IF_TK "si"
#define ELSE_TK "obien"
#define FUNC_TK "func"
#define RETURN_TK "devolver"
#define FOR_TK "para"
#define FOREACH_TK "paracada"
#define FORANGE_TK "parango"
#define WHILE_TK "mientras"
#define LOOP_TK "repetir"
#define BREAK_TK "romper"
#define CONTINUE_TK "continuar"
#define MODULE_TK "modulo"
#define IMPORT_TK "importar"
#define DEFINE_TK "definir"
#define AS_TK "como"
#define ENUM_TK "enum"
#define CLASS_TK "clase"
#define NEW_TK "nuevo"
#define STATIC_TK "estatico"
#define INIT_TK "iniciar"
#define NULL_TK "nulo"
#define DEBUG_TK "debug"

//Exceptions
#define END_COMMENT_EXCP "Falta el final del comentario: */"
#define END_STRING_EXCP "Falta el final de la cadena: "
#define UNKWN_CHAR_EXCP "Caracter desconocido '"

#endif //ODO_LEXER_ES_H
