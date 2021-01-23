#include <iostream>
#include <IO/io.h>
#include "Exceptions/exception.h"

#include "Interpreter/Interpreter.h"

#include "Modules/IOModule.h"
#include "Modules/MathModule.h"

#include"Translations/lang.h"

// This is temporary. I'll research how to bring it back.
#define RESET   ""//"\033[0m"
#define RED     ""//"\033[31m"      /* Red */
#define GREEN   ""//"\033[32m"      /* Green */
#define YELLOW  ""//"\033[33m"        /* Yellow */
//#define BLUE    "\033[34m"      /* Blue */

#define ODO_VERSION "v0.4-beta"

template<typename T> void add_module(Odo::Interpreting::Interpreter& inter) {
    inter.add_module(std::make_shared<T>(inter));
}

int main(int argc, char* argv[]) {
    std::string logo =
#if DEBUG_MODE
"               debug\n"
"            (" __DATE__ "\n"
"              " __TIME__ ")\n"
#else
"              release\n"
#endif
#if LANG_USE_ES
"               espanol\n"
#else
"               english\n"
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

        odo(-lang) )" ODO_VERSION R"(
      Luis Gonzalez (louis1001)
             2019-2020

    )";
    using namespace Odo;

    std::string input_file;

    if (argc > 1) {
        input_file = argv[1];
    }

    Interpreting::Interpreter inter;

    // Investigate what happens when adding two modules with the same name
    add_module<Modules::IOModule>(inter);
    add_module<Modules::MathModule>(inter);

    // If there's a file to be read
    if (!input_file.empty()) {
        // Opening file and reading contents:
        std::string code;
        try{
            code = Odo::io::read_file(input_file);
        } catch (Odo::Exceptions::IOException& e) {
            std::cout << std::endl;
            std::cerr << e.msg() << "\n" << std::flush;
            exit(1);
        }

        // Handle potential errors
        // Interpreting the text inside the file.

        try {
            inter.interpret(code);
        } catch(Odo::Exceptions::OdoException& e) {
            std::cout << std::endl << RED;
            auto& calls = inter.get_call_stack();
            if (e.should_show_traceback()) {
                for (const auto& frame : calls) {
                    std::cout << MSG_LINE_TXT " " << frame.line_number << ", " MSG_COL_TXT " " << frame.column_number << IN_TXT << frame.name << "\n";
                }
            }
            calls.clear();

            std::cerr << e.msg() << RESET << std::flush;
            exit(1);
        }
    // Else
    } else {
        bool continuing = true;

        inter.add_function(EXIT_FN, [&continuing](){
            continuing = false;
        });

        inter.add_function(ABOUT_FN, [logo]() {
            std::cout << logo << std::endl;
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
                        std::cout << MSG_LINE_TXT " " << frame.line_number << ", " MSG_COL_TXT  " " << frame.column_number << " " IN_TXT " " << frame.name << "\n";
                    }
                    calls.clear();
                }

                std::string msg = e.msg();
                std::cout << msg << RESET << std::endl;
            }
        }
        std::cout << BYE_MSG "! :)\n";
    }
    return 0;
}
