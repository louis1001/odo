#include <iostream>
#include <signal.h>
#include <IO/io.h>
#include "Exceptions/exception.h"

#include "Interpreter/Interpreter.h"

#include "Modules/IOModule.h"
#include "Modules/MathModule.h"

#include "external/rang.hpp"
#include "external/flags.h"

#include"Translations/lang.h"

// This is temporary. I'll research how to bring it back.
//#define RESET   ""//"\033[0m"
//#define RED     ""//"\033[31m"      /* Red */
//#define GREEN   ""//"\033[32m"      /* Green */
//#define YELLOW  ""//"\033[33m"        /* Yellow */
//#define BLUE    "\033[34m"      /* Blue */

#define ODO_VERSION "v0.4-beta"

template<typename T> void add_module(Odo::Interpreting::Interpreter& inter) {
    inter.add_module(std::make_shared<T>(inter));
}

int entry(int argc, char* argv[]) {
    auto args = flags::args(argc, argv);

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

    if (args.get<bool>("version", false) || args.get<bool>("v", false)) {
        std::cout << "odo " << ODO_VERSION;
        return 0;
    }

    if (args.get<bool>("about", false)) {
        std::cout << logo;
        return 0;
    }

    const auto& pos_args = args.positional();

    std::string input_file;

    if (pos_args.size() == 1) {
        input_file = pos_args.at(0);
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
            return 1;
        }

        // Handle potential errors
        // Interpreting the text inside the file.

        try {
            inter.interpret(code);
        } catch(Odo::Exceptions::OdoException& e) {
            std::cout << std::endl << rang::fg::red;
            auto& calls = inter.get_call_stack();
            if (e.should_show_traceback()) {
                for (const auto& frame : calls) {
                    std::cout << MSG_LINE_TXT " " << frame.line_number << ", " MSG_COL_TXT " " << frame.column_number << IN_TXT << frame.name << "\n";
                }
            }
            calls.clear();

            std::cerr << e.msg() << rang::style::reset << std::flush;
            return 1;
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
            std::cout << rang::fg::yellow;
            std::getline(std::cin, code);
            std::cout << rang::style::reset;

            // Handle potential errors
            try {
                // Evaluate the input
                auto result = inter.eval(code);

                // Show the result
                if (result != inter.get_null()){
                    auto as_str = result->to_string();
                    std::cout << rang::fg::green << as_str << rang::style::reset << "\n";
                }

                inter.set_repl_last(result);
            } catch (Odo::Exceptions::OdoException& e) {
                std::cout << std::endl << rang::fg::red;
                if (e.should_show_traceback()) {
                    auto& calls = inter.get_call_stack();
                    for (const auto& frame : calls) {
                        std::cout << MSG_LINE_TXT " " << frame.line_number << ", " MSG_COL_TXT  " " << frame.column_number << " " IN_TXT " " << frame.name << "\n";
                    }
                    calls.clear();
                }

                std::string msg = e.msg();
                std::cout << msg << rang::style::reset << std::endl;
            }
        }
        std::cout << BYE_MSG "! :)\n";
    }
    return 0;
}

int main(int argc, char* argv[]) {
    auto exit_cleanup = [](int e){
        std::cout << rang::style::reset << std::flush;
        exit(e);
    };
    // Taken from this answer https://stackoverflow.com/a/33668651
    //^C
    signal(SIGINT, exit_cleanup);
    //abort()
    signal(SIGABRT, exit_cleanup);
    //sent by "kill" command
    signal(SIGTERM, exit_cleanup);

    try {
        // Actual program!
        int result = entry(argc, argv);
        exit_cleanup(result);
        return result;
    } catch(std::exception& e) {
        exit_cleanup(1);
        throw e;
    }
}