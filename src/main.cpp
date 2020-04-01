#include <iostream>
#include "Lexer/lexer.hpp"
#include "Parser/parser.h"

#include "Interpreter/Interpreter.h"

int main(int argc, char* argv[]) {
    std::string co = R"code(
    # Dude... I completly forgot about  *continue*
    for(int i = 0; i < 10; i++) {
        println("Iteracion ", i)
    }

    int i = 0;
    while i < 20 {
        println("While ", i)
        i = i+1
    }

    int j = 0;
    loop {
        println("Looping ", j)
        j = j+1

        if j > 30 {
            break
        }
    }

    print(factorial(30))
    )code";

    if (argc > 1) {
        co = argv[1];
    }

    Lexer actual_lex("");
    Parser actual_parser(actual_lex);

    actual_parser.set_text(co);
    auto result = actual_parser.program();

    Interpreter testing_badly;

    std::cout << "Code:\n" << co << "\n\n";
    std::cout << "***************************************************\n";

    testing_badly.visit(result);

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
