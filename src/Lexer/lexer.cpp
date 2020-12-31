#include "Lexer/lexer.hpp"

#include "Exceptions/exception.h"

#include <utility>

#include "Translations/lang.h"
#define NULLCHR '\0'

namespace Odo::Lexing {
    Lexer::Lexer(std::string txt): text(std::move(txt)) {
        current_pos = static_cast<unsigned int>(-1);
        current_char = NULLCHR;
        current_line = 1;
        line_start = (int)current_pos;
        advance();
    }

    const std::map<std::string, Token> Lexer::reservedKeywords = {
            {TRUE_TK, Token(BOOL, TRUE_TK)},
            {FALSE_TK, Token(BOOL, FALSE_TK)},
            {AND_TK, Token(AND, AND_TK)},
            {OR_TK, Token(OR, OR_TK)},
            {IF_TK, Token(IF, IF_TK)},
            {ELSE_TK, Token(ELSE, ELSE_TK)},
            {FUNC_TK, Token(FUNC, FUNC_TK)},
            {RETURN_TK, Token(RET, RETURN_TK)},
            {FOR_TK, Token(FOR, FOR_TK)},
            {FOREACH_TK, Token(FOREACH, FOREACH_TK)},
            {FORANGE_TK, Token(FORANGE, FORANGE_TK)},
            {WHILE_TK, Token(WHILE, WHILE_TK)},
            {LOOP_TK, Token(LOOP, LOOP_TK)},
            {BREAK_TK, Token(BREAK, BREAK_TK)},
            {CONTINUE_TK, Token(CONTINUE, CONTINUE_TK)},
            {MODULE_TK, Token(MODULE, MODULE_TK)},
            {IMPORT_TK, Token(IMPORT, IMPORT_TK)},
            {DEFINE_TK, Token(DEFINE, DEFINE_TK)},
            {AS_TK, Token(AS, AS_TK)},
            {ENUM_TK, Token(ENUM, ENUM_TK)},
            {CLASS_TK, Token(CLASS, CLASS_TK)},
            {NEW_TK, Token(NEW, NEW_TK)},
            {STATIC_TK, Token(STATIC, STATIC_TK)},
            {INIT_TK, Token(INIT, INIT_TK)},
            {NULL_TK, Token(NULLT, NULL_TK)},
            {DEBUG_TK, Token(DEBUG, DEBUG_TK)},
    };

    void Lexer::advance() {
        current_pos++;

        if (current_pos < text.length()) {
            if (current_char == '\n') {
                current_line++;
                line_start = (int)current_pos;
            }
            current_char = text.at(current_pos);
        } else {
            current_char = NULLCHR;
        }
    }

    void Lexer::ignoreWhitespace() {
        while (
                current_char != NULLCHR &&
                isspace(current_char) &&
                current_char != '\n'
                ) {
            advance();
        }
    }

    void Lexer::ignoreMulticomment() {
        while (current_char != NULLCHR) {
            if (current_char == '}') {
                advance();
                if (current_char == '#') {
                    advance();
                    break;
                }
            } else if (current_char == '#') {
                advance();
                if (current_char == '{') {
                    ignoreMulticomment();
                }
            }

            advance();
            if (current_char == NULLCHR){
                std::string err_msg = END_COMMENT_EXCP;
                throw Exceptions::SyntaxException(err_msg, current_line, getCurrentCol());
            }
        }
    }

    void Lexer::ignoreComment() {
        while (!(current_char == NULLCHR || current_char == '\n')) {
            advance();
        }
    }

    Token Lexer::number() {
        std::string result;

        bool foundPoint = false;

        do
        {
            if (current_char == '.') {
                foundPoint = true;
            }

            result += current_char;
            advance();
        } while (
                isdigit(current_char) ||
                (current_char == '.' &&
                 !foundPoint)
                );

        // NOLINTNEXTLINE
        if (foundPoint) {
            return Token(REAL, result);
        } else {
            return Token(INT, result);
        }
    }

    Token Lexer::id() {
        std::string result;

        do
        {
            result += current_char;
            advance();
        } while (
                current_char != NULLCHR &&
                (isAlpha(current_char) ||
                 isdigit(current_char))
                );

        auto kw = reservedKeywords.find(result);

        if (kw != reservedKeywords.end()) {
            return kw->second;
        } else {
            return Token(ID, result);
        }
    }

    char Lexer::escape_char() {
        const static std::map<char, char> escape = {
                {'\\'   , '\\'},
                {'n'    , '\n'},
                {'r'    , '\r'},
                {'t'    , '\t'},
                {'b'    , '\b'},
                {'a'    , '\a'},
                {'v'    , '\v'},
                {'\''   , '\''},
                {'\"'   , '\"'},
                {'?'    , '\?'},
        };

        auto found_in_dict = escape.find(current_char);
        advance();
        if (found_in_dict != escape.end()) {
            return found_in_dict->second;
        } else {
            return current_char;
        }
    }

    std::string Lexer::string() {
        char delimiter = current_char;
        advance();

        std::string result;

        while (current_char != delimiter){
            if (current_char == NULLCHR){
                std::string err_msg = END_STRING_EXCP + std::string(1, delimiter);
                throw Exceptions::SyntaxException(err_msg, current_line, getCurrentCol());
            }
            if (current_char == '\\') {
                advance();
                result += std::string(1, escape_char());
            } else {
                result += current_char;
                advance();
            }
        }
        advance();

        return result;
    }

    Token Lexer::getNextToken() {
        if (current_char != NULLCHR && isspace(current_char) && current_char != '\n') {
            advance();
            ignoreWhitespace();
        }

        if (current_char != NULLCHR && current_char == '#') {
            advance();
            if (current_char == '{') {
                advance();
                ignoreMulticomment();
                ignoreWhitespace();
            } else {
                ignoreComment();
            }
        }

        if (current_char == NULLCHR) {
            return Token(EOFT, "");
        }

        if (isdigit(current_char)) {
            return number();
        } else if (isAlpha(current_char)) {
            return id();
        } else {
            switch (current_char) {
                case '"':
                case '\'':
                    return Token(STR, string());
                case '+':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(PLUSE, "+=");
                    } else if (current_char == '+'){
                        advance();
                        return Token(PLUSP, "++");
                    }

                    return Token(PLUS, "+");
                case '-':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(MINUSE, "-=");
                    } else if (current_char == '-'){
                        advance();
                        return Token(MINUSP, "--");
                    } else if (current_char == '>'){
                        advance();
                        return Token(ARROW, "->");
                    }
                    return Token(MINUS, "-");
                case '*':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(MULE, "*=");
                    }
                    return Token(MUL, "*");
                case '/':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(DIVE, "/=");
                    }
                    return Token(DIV, "/");
                case '%':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(MODE, "%=");
                    }
                    return Token(MOD, "%");
                case '^':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(POWE, "^=");
                    }
                    return Token(POW, "^");
                case '!':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(NEQ, "!=");
                    }

                    return Token(NOT, "!");
                case '<':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(LET, "<=");
                    }

                    return Token(LT, "<");
                case '>':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(GET, "<=");
                    }

                    return Token(GT, "<");
                case '~':
                    advance();
                    return Token(REV, "~");
                case '(':
                    advance();
                    return Token(LPAR, "(");
                case ')':
                    advance();
                    return Token(RPAR, ")");
                case '{':
                    advance();
                    return Token(LCUR, "{");
                case '}':
                    advance();
                    return Token(RCUR, "}");
                case '[':
                    advance();
                    return Token(LBRA, "[");
                case ']':
                    advance();
                    return Token(RBRA, "]");
                case ',':
                    advance();
                    return Token(COMMA, ",");
                case ';':
                    advance();
                    return Token(SEMI, ";");
                case ':':
                    advance();
                    if (current_char == ':'){
                        advance();
                        return Token(DCOLON, "::");
                    }
                    return Token(COLON, ":");
                case '.':
                    advance();
                    return Token(DOT, ".");
                case '?':
                    advance();
                    return Token(QUEST, "?");
                case '=':
                    advance();
                    if (current_char == '='){
                        advance();
                        return Token(EQU, "==");
                    }
                    return Token(ASS, "=");
                case '\n':
                    advance();

                    return Token(NL, "\n");
                default:
                    break;
            }
        }

        std::string err_msg = UNKWN_CHAR_EXCP + std::string(1, current_char) + "'";
        throw Exceptions::SyntaxException(err_msg, current_line, getCurrentCol());

    }

    bool Lexer::isAlpha(char c) {
        return isalpha(c) || c == '_';
    }

    void Lexer::reset() {
        current_pos = static_cast<unsigned int>(-1);
        current_line = 1;
        line_start = (int)current_pos;
        advance();
    }
}