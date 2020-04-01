#include "Lexer/lexer.hpp"

#define NULLCHR '\0'

Lexer::Lexer(std::string txt): text(txt) {
    current_pos = -1;
    advance();
}

const std::map<std::string, Token> Lexer::reservedKeywords = {
    {"true", Token(::BOOL, "true")},
    {"false", Token(::BOOL, "false")},
    {"if", Token(::IF, "if")},
    {"else", Token(::ELSE, "else")},
    {"func", Token(::FUNC, "func")},
    {"return", Token(::RET, "return")},
    {"for", Token(::FOR, "for")},
    {"while", Token(::WHILE, "while")},
    {"loop", Token(::LOOP, "loop")},
    {"break", Token(::BREAK, "break")},
    {"class", Token(::CLASS, "class")},
    {"new", Token(::NEW, "new")},
    {"static", Token(::STATIC, "static")},
    {"init", Token(::INIT, "init")},
    {"null", Token(::NULLT, "null")},
    {"debug", Token(::DEBUG, "debug")},
};

void Lexer::advance() {
    current_pos++;
    
    if (current_pos < text.length()) {
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
        if (current_char == '*') {
            advance();
            if (current_char == '/') {
                advance();
                break;
            }
        } else if (current_char == '/') {
            advance();
            if (current_char == '*') {
                ignoreMulticomment();
            }
        }

        advance();
    }
}

void Lexer::ignoreComment() {
    while (!(current_char == NULLCHR || current_char == '\n')) {
        advance();
    }
}

Token Lexer::number() {
    std::string result = "";

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

std::string Lexer::string() {
    char delimiter = current_char;
    advance();

    std::string result;

    while (current_char != delimiter){
        result += current_char;
        advance();
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
        ignoreComment();
    }

    if (current_char == '/') {
        advance();
        if (current_char == '*') {
            advance();
            ignoreMulticomment();
        } else {
            current_pos -= 2;
            advance();
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
                return Token(STR, string());
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
            case '/':
                advance();
                return Token(DIV, "/");
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
            case '&':
                advance();
                if (current_char == '&'){
                    advance();
                    return Token(AND, "&&");
                }
            case '|':
                advance();
                if (current_char == '|'){
                    advance();
                    return Token(OR, "||");
                }
            case '\n':
                advance();
                current_line += 1;
                line_start = current_pos;

                return Token(NL, "\n");
            default:
                break;
        }
    }

    throw 1;
}

bool Lexer::isAlpha(char c) {
    return isalpha(c) || c == '_';
}

void Lexer::reset() {
    current_pos = -1;
    advance();
}