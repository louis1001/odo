#include <iostream>
#include "Lexer/lexer.hpp"
#include "Parser/parser.h"

#include "Interpreter/Interpreter.h"

int main(int argc, char* argv[]) {
    std::string co = R"code(
# Find all fibonacci numbers up to n and store them in a list
int n = 1000

int a = 1
int b = 0
int c = 0

int results[] = []

while c < n {
    b = a
    a = c
    c = a + b

    results += c
}

for(int i = 0; i < length(results); i++) {
    println("[", i, "]: ", results[i])
}

println()
println("Testing a separator")
println("v^"*50)

println("Testing a list with some repeating values")
println([0]*20)
    )code";

    if (argc > 1) {
        co = argv[1];
    }

    Lexer actual_lex("");
    Parser actual_parser(actual_lex);

    actual_parser.set_text(co);
    auto result = actual_parser.program();

    Interpreter testing_badly;

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
