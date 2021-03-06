#pragma once
#include <string>
#include <map>

#include "Lexer/token.hpp"

namespace Odo::Lexing {
    class Lexer {
        const static std::map<std::string, Token> reservedKeywords;

        unsigned int current_pos = 0;
        int current_line = 1;
        int line_start = 0;

        char current_char;

        static bool isAlpha(char c);

        char escape_char();

        void ignoreWhitespace();
        void ignoreMulticomment();
        void ignoreComment();

        Token number();
        Token id();
        std::string string();

    public:
        std::string text;
        Lexer(std::string text = "");
        void advance();
        Token getNextToken();
        void reset();

        unsigned int getCurrentLine() { return current_line; }
        unsigned int getCurrentCol() { return current_pos - line_start; }
    };
}