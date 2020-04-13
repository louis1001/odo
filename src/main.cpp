#include <iostream>
#include "Lexer/lexer.hpp"
#include "Parser/parser.h"
#include "Exceptions/exception.h"

#include <fstream>
#include <sstream>

#include "Interpreter/Interpreter.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"        /* Yellow */
#define BLUE    "\033[34m"      /* Blue */

std::string readWhole(std::istream& in) {
    // Taken from https://stackoverflow.com/a/116220
    std::stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

int main(int argc, char* argv[]) {
    using namespace Odo;

    std::string input_file;

    if (argc > 1) {
        input_file = argv[1];
    }


    Interpreting::Interpreter inter;

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

        try {
            inter.interpret(code);
        } catch(Odo::Exceptions::Exception& e) {
            std::cout << std::endl;
            std::cerr << e.what() << "\n" << std::flush;
        }
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
            std::cout << "> " << std::flush;
            std::string code;
            std::cout << YELLOW;
            std::getline(std::cin, code);
            std::cout << RESET;

            // Handle potential errors
            try {
                // Evaluate the input
                auto result = inter.eval(code);

                // Show the result
                if (result != inter.get_null()){
                    auto as_str = inter.value_to_string(*result);
                    std::cout << GREEN << as_str << RESET << "\n";
                }
            } catch (Odo::Exceptions::Exception& e) {
                std::cout << std::endl;
                std::string msg = e.what();
                std::cout << RED << msg << RESET << std::endl;
            }
        }
        std::cout << "Bye! :)\n";
    }
    return 0;
}
