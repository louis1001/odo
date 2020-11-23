#include <iostream>
#include <IO/io.h>
#include "Exceptions/exception.h"

#include "Interpreter/Interpreter.h"

// This is temporary. I'll research how to bring it back.
#define RESET   ""//"\033[0m"
#define RED     ""//"\033[31m"      /* Red */
#define GREEN   ""//"\033[32m"      /* Green */
#define YELLOW  ""//"\033[33m"        /* Yellow */
//#define BLUE    "\033[34m"      /* Blue */

int main(int argc, char* argv[]) {
    std::string logo =
#if DEBUG_MODE
"               debug"
#else
"              release"
#endif
    R"(
          (((((((((((((((
       (((((((((((((((((((((
     (((((((           ******
     ((((((             ******
     ((((((             **   *
     ((((((             ******
     ((((((((         *******
       (((((((((((((((((((((
          (((((((((((((((

        odo(-lang) v0.3-beta
      Luis Gonzalez (louis1001)
             2019-2020

    )";
    using namespace Odo;

    std::string input_file;

    if (argc > 1) {
        input_file = argv[1];
    }

    Interpreting::Interpreter inter;

    // If there's a file to be read
    if (!input_file.empty()) {
        // Opening file and reading contents:
        std::string code;
        try{
            code = Odo::io::read_file(input_file);
        } catch (Odo::Exceptions::IOException& e) {
            std::cout << std::endl;
            std::cerr << e.what() << "\n" << std::flush;
            exit(1);
        }

        // Handle potential errors
        // Interpreting the text inside the file.

        try {
            inter.interpret(code);
        } catch(Odo::Exceptions::OdoException& e) {
            std::cout << std::endl << RED;
            if (e.should_show_traceback()) {
                auto& calls = inter.get_call_stack();
                for (const auto& frame : calls) {
                    std::cout << "line " << frame.line_number << ", column " << frame.column_number << " in " << frame.name << "\n";
                }
                calls.clear();
            }

            std::cerr << e.what() << RESET << std::flush;
            exit(1);
        }
    // Else
    } else {
        bool continuing = true;

        inter.add_native_function("exit", [&](auto vals){
            continuing = false;
            return inter.get_null();
        });

        inter.add_native_function("about", [&](auto) {
            std::cout << logo << std::endl;
            return inter.get_null();
        });

        // NOLINTNEXTLINE
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
                    auto as_str = result->to_string();
                    std::cout << GREEN << as_str << RESET << "\n";
                }

                inter.set_repl_last(result);
            } catch (Odo::Exceptions::OdoException& e) {
                std::cout << std::endl << RED;
                if (e.should_show_traceback()) {
                    auto& calls = inter.get_call_stack();
                    for (const auto& frame : calls) {
                        std::cout << "line " << frame.line_number << ", column " << frame.column_number << " in " << frame.name << "\n";
                    }
                    calls.clear();
                }

                std::string msg = e.what();
                std::cout << msg << RESET << std::endl;
            }
        }
        std::cout << "Bye! :)\n";
    }
    return 0;
}
