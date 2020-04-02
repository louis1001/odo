#include <iostream>
#include "Lexer/lexer.hpp"
#include "Parser/parser.h"

#include <fstream>
#include <sstream>

#include "Interpreter/Interpreter.h"

std::string readWhole(std::istream& in) {
    // Taken from https://stackoverflow.com/a/116220
    std::stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

int main(int argc, char* argv[]) {
    std::string input_file;

    if (argc > 1) {
        input_file = argv[1];
    }

    Interpreter inter;

    // If there's a file to be read
    if (!input_file.empty()) {
        // Opening file and reading contents:
        std::ifstream argumentFile(input_file);
        if (argumentFile.fail()) {
            std::cerr << "Unable to open file '" << input_file << "'.";
        }
        std::string code = readWhole(argumentFile);

        argumentFile.close();

        // Handle potential errors
        // Interpreting the text inside the file.
//      try {
        inter.interpret(code);
//      }
    // Else
    } else {
        bool continuing = true;

        inter.add_native_function("exit", [&](auto vals){
            continuing = false;
            return inter.get_null();
        });

        while (continuing)
        {
            // Show an interactive prompt
            std::cout << "> ";
            std::string code;
            std::getline(std::cin, code);

            // Handle potential errors
//        try
            // Evaluate the input
            auto result = inter.eval(code);

            // Show the result
            std::cout << "\n" << inter.value_to_string(*result) << "\n";
//        }
        }
        std::cout << "Bye! :)\n";
    }
    return 0;
}
