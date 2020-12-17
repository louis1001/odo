//
// Created by admin on 16/12/2020.
//

#ifndef ODO_LANG_H
#define ODO_LANG_H

#if LANG_USE_ES
#include "Translations/lexer_es.h"
#include "Translations/parser_es.h"
#include "Translations/exception_es.h"
#else
#include "Translations/lexer_en.h"
#include "Translations/parser_en.h"
#include "Translations/exception_en.h"
#endif

#endif //ODO_LANG_H
