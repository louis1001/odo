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
#define INVALID_ASS_TYPE_EXCP "Asignacion invalida. Asignando a un simbolo de tipo "

#endif //ODO_SEMANTICANALYZER_ES_H
