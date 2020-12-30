//
// Created by louis1001 on 20/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_SEMANTICANALYZER_ES_H
#define ODO_SEMANTICANALYZER_ES_H
#define NO_VALUE_TO_INDEX_EXCP "Uso del operador de indice cuando no hay valor que indexar."
#define BOTH_BRANCH_SAME_TYPE_EXCP "Ambas ramas del operador ternario deben devolver el mismo tipo."
#define BRANCHES_MUST_RETURN_EXCP "Las ramas del operador ternario deben ser expresiones validas. (Deben devolver un valor)"
#define LST_EL_NO_VALUE_EXCP "El elemento en la lista no tiene o devuelve valor."
#define NOTHING_TO_ITERATE_EXCP "No hay nada sobre que iterar en la sentencia 'paracada'"
#define INVALID_DECL_TYPE_EXCP "Declaracion invalida. Inicializando variable de tipo "
#define WITH_VAL_OF_TYPE_EXCP " con un valor de tipo "
#define VAR_INIT_MUST_BE_VALID_EXCP "La inicializacion de una variable debe ser una expresion valida (Debe devolver un valor)"
#define INVALID_LIST_INIT_NOT_LIST_EXCP "La inicializacion de lista no es valida. El valor debe ser una lista."
#define INVALID_LST_DECL_TYPE_EXCP "Declaracion de lista invalida. Inicializando lista de tipo "
#define WITH_LST_VAL_OF_TYPE_EXCP " con valor de tipo "
#define LST_INIT_MUST_BE_VALID_EXCP "La inicializacion de una lista debe ser una expresion valida (Debe devolver un valor)"
#define USI_VAR_NOT_INIT_EXCP "Usando una variable que no ha sido inicializada."
#define SYM_NOT_DEFINED_EXCP "No se puede conseguir una variable miembro de este simbolo. No ha sido definido."
#define INDEX_MUST_BE_VALID_EXCP "El valor en el operdor de indice debe ser una expresion valida de tipo ent."
#define INVALID_ASS_SYMBOL_NOT_EXCP "Asignacion invalida. El simbolo al lado izquierdo no es asignable."
#define INVALID_ASS_TYPE_EXCP "Asignacion invalida. Asignando a un simbolo de tipo "

#define RIGHT_OP_BIN_NO_VAL_EXCP "El operando derecho en la operacion binaria no tiene un valor."
#define LEFT_OP_BIN_NO_VAL_EXCP "El operando derecho en la operacion binaria no tiene un valor."

#define CONC_LST_INCOM_TPS_EXCP "Cocatenando listas de tipos incompatibles."
#define APP_LST_INCOM_TP_EXCP "Agregando a una lista un valor de tipo incompatible."
#define LST_ONLY_MUL_INT_EXCP "Listas solo pueden ser multiplicadas por enteros."
#define STR_ONLY_MUL_INT_EXCP "Cadenas solo puede ser multiplicadas por enteros."

#define INVALID_CALL_ARG_EXCP "Llamada de funcion invalida. Todos los argumentos deben ser expresiones validas."

#define RET_OUTS_FUNC_BODY_EXCP "Sentencia 'devolver' fuera del cuerpo de una funcion."
#define RET_ON_VOID_FUNC_EXCP "Devolviendo un valor dentro de una funcion sin tipo de retorno."
#define RET_VAL_TYPE_EXCP "El valor devuelto de tipo '"
#define NOT_MATCH_FUNC_EXCP "' no concuerda con el tipo de la funcion: '"

#define UNEXP_CALL_NOT_FUNC_EXCP "Llamada de funcion invalida. La sentencia no tiene un valor al que llamar."
#define FUNC_OF_TP_EXCP "La funcion de tipo '"
#define TAKES_EXCP "' toma como maximo "
#define ARGS_BUT_CALLED_EXCP " argumentos, pero fue llamada con "
#define INVALID_TP_FOR_ARG_EXCP "Tipo invalido para argumento en llamada de funcion. El argumento "
#define EXPC_TP_EXCP " esperaba un valor de tipo "
#define BUT_RECVD_EXCP " pero recibio "
#define NO_VAL_FOR_FUNC_ARG_EXCP "La llamada a la funcion no tiene valor para el argumento "
#define CANT_DEFINE_NON_OPT_AFTER_OPT_EXCP "No puede definirse un argumento de funcion no opcional despues de uno opcional."

#define UNKNWN_CLASS_EXCP "Clase desconocida: '"

#define CONSTR_CALL_TAKES_EXCP "El constructor toma como maximo "
#define INVALID_TP_FOR_CONSTR_ARG_EXCP "Tipo invalido para argumento en llamada de constructor. El argumento "
#define NO_VAL_FOR_CONSTR_ARG_EXCP "La llamada al constructor no tiene valor para el argumento "

#define USE_OF_STA_EXCP "Uso de sentencia '"
#define OUTS_A_LOOP_EXCP "' fuera de un ciclo."

#define UNKNWN_TP_IN_FUNC_DEF_EXCP "Simbolo desconocido en la definicion de un tipo de funcion: "
#define SYM_IN_FUNC_DEF_EXCP "El simbolo en la definicion de tipo de funcion '"
#define IS_NOT_TP_EXCP "' no es un tipo de valor."

#endif //ODO_SEMANTICANALYZER_ES_H
