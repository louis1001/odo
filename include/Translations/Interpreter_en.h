//
// Created by louis1001 on 17/12/2020.
// Copyright (c) 2020 louis1001. All rights reserved.
//

#ifndef ODO_INTERPRETER_EN_H
#define ODO_INTERPRETER_EN_H

// Type Names
#define ANY_TP "any"
#define INT_TP "int"
#define DOUBLE_TP "double"
#define STRING_TP "string"
#define BOOL_TP "bool"
#define POINTER_TP "pointer"

// Variables
#define NULL_TP "NullType"
#define THIS_VAR "this"

// Global Functions
#define WRITE_FN "write"
#define WRITELN_FN "writeln"
#define MOVE_CRSR_FN "move_cursor"
#define FACTR_FN "factorial"
#define LENGTH_FN "length"
#define FROM_ASCII_FN "fromAsciiCode"
#define TO_ASCII_FN "toAsciiCode"
#define POW_FN "pow"
#define SQRT_FN "sqrt"
#define SIN_FN "sin"
#define COS_FN "cos"
#define FLOOR_FN "floor"
#define TRUNC_FN "trunc"
#define ROUND_FN "round"
#define READ_FN "read"
#define READ_INT_FN "read_int"
#define READ_DOUBLE_FN "read_double"
#define RAND_FN "rand"
#define RAND_INT_FN "randInt"
#define POP_FN "pop"
#define PUSH_FN "push"
#define TYPEOF_FN "typeof"
#define CLEAR_FN "clear"
#define WAIT_FN "wait"
#define SLEEP_FN "sleep"

// Exceptions
#define NOT_IMPL_EXCP "Using function not yet implemented: "
#define FACTR_REQ_INT_EXCP "factorial function requires a single int argument."
#define LENGTH_REQ_ARGS_EXCP "length function requires a single argument of type string or list."
#define FLOOR_ONLY_NUM_EXCP "floor function can only be called with numeric values"
#define TRUNC_ONLY_NUM_EXCP "trunc function can only be called with numeric values"
#define ROUND_ONLY_NUM_EXCP "round function can only be called with numeric values and an optional int"
#define STATIC_ONLY_CLASS_EXCP "Static statements can only appear inside a class body."
#define INVALID_BOOL_EXCP "Invalid value for bool expression."
#define COND_TERN_MUST_BOOL_EXCP "Condition of ternary expression must be boolean."
#define COND_IF_MUST_BOOL_EXCP "Condition of if statement must be boolean."
#define COND_FOR_MUST_BOOL_EXCP "Condition expression of for statement must be boolean."
#define COND_WHILE_MUST_BOOL_EXCP "Condition expression of for statement must be boolean."
#define FOREACH_ONLY_LIST_STR_EXCP "foreach statement can only be used with list or string values."
#define VAL_RANGE_NUM_EXCP "Values defining the range of forange statement have to be numerical"
#define VAR_CALLED_EXCP "Variable called '"
#define ALR_EXISTS_EXCP "' already exists"
#define NOT_DEFINED_EXCP "' not defined."
#define UNKWN_TYPE_EXCP "Unknown type '"
#define INVALID_TYPE_EXCP "Invalid type '"
#define ASS_TO_UNKWN_VAR_EXCP "Assignment to unknwon variable."
#define INDX_STR_OB_EXCP "Indexing a string out of bounds."
#define STR_ONLY_INDX_NUM_EXCP "Strings can only be indexed with integer values."
#define INDX_LST_OB_EXCP "Indexing a list out of bounds."
#define LST_ONLY_INDX_NUM_EXCP "Lists can only be indexed with integer values."
#define INDX_ONLY_LST_STR_EXCP "Index operator is only valid for strings and lists values."
#define ADD_ONLY_SAME_TP_EXCP "Addition operation can only be used with values of the same type."
#define SUB_ONLY_SAME_TP_EXCP "Numeric substraction can only be used with values of the same type."
#define MUL_ONLY_SAME_TP_EXCP "Multiplication operation can only be used with values of the same type."
#define DIV_ONLY_DOB_EXCP "Division operation can only be used with values of type double."
#define MOD_ONLY_INT_EXCP "Modulo operation can only be used with values of type int."
#define POW_ONLY_SAME_TP_EXCP "Power operation can only be used with values of the same type."
#define COM_ONLY_SAME_TP_EXCP "Comparison operation can only be used with values of same type."
#define LOG_ONLY_BOOL_EXCP "Logical operator can only be used with values of type bool."
#define UNA_ONLY_NUM_EXCP "Unary operator can be used with int or double values."
#define CALL_DEPTH_EXC_EXCP "Callback depth exceeded."
#define VAL_NOT_FUNC_EXCP "Value is not a function."
#define CLASS_MUST_INH_TYPE_EXCP "Class must inherit from another type. "
#define IS_INVALID_EXCP " is invalid."
#define INVALID_CONS_EXCP "Invalid constructor"
#define NOT_VALID_CONS_EXCP " is not a valid constructor."
#define INVALID_INS_MEM_OP_EXCP "Invalid instance for Member Variable operator."
#define NO_MEM_CALLED_EXCP "No member variable named '"
#define IN_THE_INST_EXCP "' in the Instance."
#define NO_STATIC_CALLED_EXCP "No static variable named '"
#define IN_CLASS_EXCP "' in class."
#define NO_VAR_NAMED_EXCP "No variable named '"
#define IN_MODULE_EXCP "' in module."
#define NOT_VARIANT_IN_ENUM_EXCP "' is not a variant in enum."
#define CANNOT_READ_STATIC_EXCP "Cannot read static variable from this value."
#define NOT_VALID_INST_EXCP "' is not a valid instance."
#define INVALID_STATIC_OP_EXCP "Invalid Static Variable Operator (::)."
#define UNKWN_VAL_IN_STATIC_EXCP "Unknown value in Static Variable Operator (::)."
#define ASS_TO_INVALID_INDX_EXCP "Assignment to invalid indexing. You can only assign to list indices."
#define SYM_CALLED_EXCP "Symbol called "
#define ALR_EXISTS_IN_SCOPE_EXCP " already exists in this scope"
#define CANNOT_IMPORT_MODULE_EXCP "Cannot import module '"
#define EXPCT_DECL_IN_PAR_EXCP "Expected parameter declaration in function parenthesis"

#define READ_FILE_FN "read_file"
#define TO_ABS_PATH_FN "to_absolute_path"
#define IS_DIR_FN "is_dir"
#define IS_FILE_FN "is_file"
#define LIST_DIR_FN "list_dir"
#define CREATE_FILE_FN "create_file"
#define WRITE_TO_FILE_FN "write_to_file"
#define APPEND_TO_FILE_FN "append_to_file"
#define COULD_NOT_READ_EXCP "Could not read the file '"
#define MAY_NOT_EXIST_EXCP "'. It may not exist, or you don't have permission to open it."
#define COULD_NOT_WRITE_EXCP "Could not write to file '"
#define FOL_MAY_NOT_EXIST_EXCP "'. The folder may not exist, or you don't have permission to modify it."
#define GET_CWD_FN "get_cwd"
#define PATH_EXISTS_FN "path_exists"

#endif //ODO_INTERPRETER_EN_H
