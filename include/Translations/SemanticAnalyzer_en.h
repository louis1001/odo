//
// Created by louis1001 on 20/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_SEMANTICANALYZER_EN_H
#define ODO_SEMANTICANALYZER_EN_H
#define NO_VALUE_TO_INDEX_EXCP "Using index operator where there's no value to index."
#define BOTH_BRANCH_SAME_TYPE_EXCP "Both branches in ternary operator must return the same type."
#define BRANCHES_MUST_RETURN_EXCP "Ternary operator branches must be valid expressions. (Must return value)"
#define LST_EL_NO_VALUE_EXCP "Element in list has no value."
#define NOTHING_TO_ITERATE_EXCP "Nothing to iterate over in foreach statement"
#define INVALID_DECL_TYPE_EXCP "Invalid declaration. Initializing variable of type "
#define WITH_VAL_OF_TYPE_EXCP " with value of type "
#define VAR_INIT_MUST_BE_VALID_EXCP "Variable initialization must be a valid expression (Must return value)"
#define INVALID_LIST_INIT_NOT_LIST_EXCP "Invalid list initialization. The value is not a list."
#define INVALID_LST_DECL_TYPE_EXCP "Invalid list declaration. Initializing list of type "
#define WITH_LST_VAL_OF_TYPE_EXCP " with value of type "
#define LST_INIT_MUST_BE_VALID_EXCP "List initialization must be a valid expression (Must return value)"
#define USI_VAR_NOT_INIT_EXCP "Using variable that has not been initialized."
#define SYM_NOT_DEFINED_EXCP "Can't get member variable from this symbol. It hasn't been defined."
#define INDEX_MUST_BE_VALID_EXCP "Index value must be a valid expression of type integer."
#define INVALID_ASS_TYPE_EXCP "Invalid assignment. Assigning to symbol of type "

#endif //ODO_SEMANTICANALYZER_EN_H
