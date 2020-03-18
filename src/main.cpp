#include <iostream>
#include "Lexer/lexer.hpp"

int main(int argc, char* argv[]) {

    std::string co = "class Hello { func hello(){ int a = 10; } }";

    if (argc > 1) {
        co = argv[1];
    }

    Lexer a(co);

    Token cur = a.getNextToken();

    while (cur.tp != EOFT) {
        std::cout << "New Token: " << cur.value << "\n";
        cur = a.getNextToken();
    }

    // If there's a file to be read

        // Opening file and reading contents:
        // text lines.

        // Handle potential errors
            // Interpreting the text inside the file.

    // Else
        // Show an interactive prompt

        // Handle potential errors
            // Evaluate the input

            // Show the result

    return 0;
}
