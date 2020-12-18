//
// Created by louis1001 on 17/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_INTERPRETER_ES_H
#define ODO_INTERPRETER_ES_H
#include "lang.h"

// Type Names
#define ANY_TP "var"
#define INT_TP "ent"
#define DOUBLE_TP "doble"
#define STRING_TP "cadena"
#define BOOL_TP "bool"
#define POINTER_TP "puntero"

// Variables
#define NULL_TP "NullType"
#define THIS_VAR "yo"

// Global Functions
#define PRINT_FN "imprimir"
#define PRINTLN_FN "imprimirln"
#define MOVE_CRSR_FN "mover_cursor"
#define FACTR_FN "factorial"
#define LENGTH_FN "longitud"
#define FROM_ASCII_FN "deCodigoAscii"
#define TO_ASCII_FN "aCodigoAscii"

/* TODO: Not sure if I should keep them in english
 *   Because of conventions with calculators
 */
#define POW_FN "pow"
#define SQRT_FN "sqrt"
#define SIN_FN "sin"
#define COS_FN "cos"
#define FLOOR_FN "floor"
#define TRUNC_FN "trunc"
#define ROUND_FN "round"

#define READ_FN "leer"
#define READ_INT_FN "leer_ent"
#define READ_DOUBLE_FN "leer_doble"
#define RAND_FN "azar"
#define RAND_INT_FN "azarEnt"
#define POP_FN "retirar"
#define PUSH_FN "apilar"
#define TYPEOF_FN "tipode"
#define CLEAR_FN "limpiar"
#define WAIT_FN "esperar"
#define SLEEP_FN "dormir"

// Exceptions
#define NOT_IMPL_EXCP "Usando una funcion que aun no ha sido implementada: "
#define FACTR_REQ_INT_EXCP "La funcion '" FACTR_FN "' necesita un unico argumento tipo ent."
#define LENGTH_REQ_ARGS_EXCP "La funcion '" LENGTH_FN "' necesita un unico argumento tipo lista o " STRING_TP "."
#define FLOOR_ONLY_NUM_EXCP "La funcion '" FLOOR_FN "' solo puede ser llamada con valores numericos"
#define TRUNC_ONLY_NUM_EXCP "La funcion '" TRUNC_FN "' solo puede ser llamada con valores numericos"
#define ROUND_ONLY_NUM_EXCP "La funcion '" ROUND_FN "' solo puede ser llamada con valores numericos y un ent opcional"
#define STATIC_ONLY_CLASS_EXCP "Sentencias estaticas solo pueden aparecer dentro del cuerpo de una clase."
#define INVALID_BOOL_EXCP "Valor invalido para una expresion tipo '" BOOL_TP "'."
#define COND_TERN_MUST_BOOL_EXCP "La condicion de una expresion ternaria debe tener tipo '" BOOL_TP "'."
#define COND_IF_MUST_BOOL_EXCP "La condicion de una sentencia '" IF_TK "' debe tener tipo '" BOOL_TP "'."
#define COND_FOR_MUST_BOOL_EXCP "La condicion de una sentencia '" FOR_TK "' debe tener tipo '" BOOL_TP "'."
#define COND_WHILE_MUST_BOOL_EXCP "La condicion de una sentencia '" WHILE_TK "' debe tener tipo '" BOOL_TP "'."
#define FOREACH_ONLY_LIST_STR_EXCP "La sentencia '" FOREACH_TK "' solo puede ser usada con valores de lista o " STRING_TP "."
#define VAL_RANGE_NUM_EXCP "Los valores que definen el rango de una sentencia '" FORANGE_TK "' deben ser numericos"
#define VAR_CALLED_EXCP "Una variable llamada '"
#define ALR_EXISTS_EXCP "' ya existe."
#define NOT_DEFINED_EXCP "' no ha sido definida."
#define UNKWN_TYPE_EXCP "Tipo desconocido '"
#define INVALID_TYPE_EXCP "Tipo invalido '"
#define ASS_TO_UNKWN_VAR_EXCP "Asignacion a variable desconocida."
#define INDX_STR_OB_EXCP "Indexando una " STRING_TP " fuera de sus limites."
#define STR_ONLY_INDX_NUM_EXCP STRING_TP " solo puede ser indexada con valores numericos."
#define INDX_LST_OB_EXCP "Indexando una lista fuera de sus limites."
#define LST_ONLY_INDX_NUM_EXCP "Listas solo pueden ser indexadas con valores numericos."
#define INDX_ONLY_LST_STR_EXCP "El operador de indice solo puede ser usado con valores de lista o " STRING_TP "."
#define ADD_ONLY_SAME_TP_EXCP "La operacion de suma solo puede ser usada con valores del mismo tipo."
#define SUB_ONLY_SAME_TP_EXCP "La operacion de resta solo puede ser usada con valores del mismo tipo."
#define MUL_ONLY_SAME_TP_EXCP "La operacion de multiplicacion solo puede ser usada con valores del mismo tipo."
#define DIV_ONLY_DOB_EXCP "La operacion de division solo puede ser usada con valores de tipo '" DOUBLE_TP "'."
#define MOD_ONLY_INT_EXCP "La operacion de modulo (resto) solo puede ser usada con valores de tipo '" INT_TP "'."
#define POW_ONLY_SAME_TP_EXCP "La operacion de poder solo puede ser usada con valores del mismo tipo."
#define COM_ONLY_SAME_TP_EXCP "La operacion de comparacion solo puede ser usada con valores del mismo tipo."
#define LOG_ONLY_BOOL_EXCP "El operador logico solo puede ser usada con valores de tipo '" BOOL_TP "'."
#define UNA_ONLY_NUM_EXCP "El operador unitario solo puede ser usada con valores de tipo numerico."
#define CALL_DEPTH_EXC_EXCP "Limite de profundidad en llamadas de funcion excedido."
#define VAL_NOT_FUNC_EXCP "El valor no es una funcion."
#define CLASS_MUST_INH_TYPE_EXCP "La clase debe heredar de un tipo."
#define IS_INVALID_EXCP " es invalido."
#define INVALID_CONS_EXCP "Constructor no valido"
#define NOT_VALID_CONS_EXCP " no es un constructor valido."
#define INVALID_INS_MEM_OP_EXCP "Instancia invalida para el operador de variable miembro."
#define NO_MEM_CALLED_EXCP "No hay una variable miembro llamada '"
#define IN_THE_INST_EXCP "' el la Instancia."
#define NO_STATIC_CALLED_EXCP "No hay una variable estatica llamada '"
#define IN_CLASS_EXCP "' el la clase."
#define NO_VAR_NAMED_EXCP "No hay una variable llamada '"
#define IN_MODULE_EXCP "' en el modulo."
#define NOT_VARIANT_IN_ENUM_EXCP "' no es una variante del " ENUM_TK "."
#define CANNOT_READ_STATIC_EXCP "No se puede leer una variable estatica en este valor."
#define NOT_VALID_INST_EXCP "' no es una Instancia valida."
#define INVALID_STATIC_OP_EXCP "El operador de variable estatica (::) no es valido."
#define UNKWN_VAL_IN_STATIC_EXCP "Valor desconocido en operador de variable estatica (::)."
#define ASS_TO_INVALID_INDX_EXCP "Asignacion a indice invalido. Solo se puede asignar a un indice en listas."
#define SYM_CALLED_EXCP "Un simbolo llamado "
#define ALR_EXISTS_IN_SCOPE_EXCP " ya existe en este contexto."
#define CANNOT_IMPORT_MODULE_EXCP "No se pudo importar el modulo '"
#define EXPCT_DECL_IN_PAR_EXCP "Se esperaba una declaracion de parametros en los parentesis de la funcion."

#endif //ODO_INTERPRETER_ES_H
