//
// Created by admin on 16/12/2020.
//

#ifndef ODO_LANG_H
#define ODO_LANG_H

#if LANG_USE_ES
    #include "Translations/lexer_es.h"
    #include "Translations/parser_es.h"
    #include "Translations/exception_es.h"
    #include "Translations/Interpreter_es.h"
    #include "Translations/value_es.h"
    #include "Translations/main_es.h"
    #include "Translations/symbol_es.h"
    #include "Translations/SemanticAnalyzer_es.h"
    #include "Translations/Modules_es.h"
#else
    #include "Translations/lexer_en.h"
    #include "Translations/parser_en.h"
    #include "Translations/exception_en.h"
    #include "Translations/Interpreter_en.h"
    #include "Translations/value_en.h"
    #include "Translations/main_en.h"
    #include "Translations/symbol_en.h"
    #include "Translations/SemanticAnalyzer_en.h"
    #include "Translations/Modules_en.h"
#endif

#endif //ODO_LANG_H
