//
// Created by Luis Gonzalez on 4/7/20.
//

#include <exception>
#include <sstream>

#ifndef ODO_PORT_EXCEPTION_H
#define ODO_PORT_EXCEPTION_H
namespace Odo::Exceptions {
    class Exception: std::exception {
        unsigned int line_number{};
        unsigned int column_number{};

        std::string message{};

        virtual std::string exception_name() { return "GenericError"; };
    public:
        explicit Exception(const std::string& msg, unsigned int line_n=0, unsigned int col_n=0) {
            message = msg;
            line_number = line_n;
            column_number = col_n;
        }

        const char* what() noexcept {
            std::string ss;
            ss += exception_name() + " (line " + std::to_string(line_number) + ", column " +
                    std::to_string(column_number) + ")\n\t" + message;
            return ss.c_str();
        }
    };

    class SyntaxException: public Exception {
        std::string exception_name() override { return "SyntaxError"; }
    public:
        using Exception::Exception;
    };

    class TypeException: public Exception {
        std::string exception_name() override { return "TypeError"; }
    public:
        using Exception::Exception;
    };

    class ValueException: public Exception {
        std::string exception_name() override { return "ValueError"; }
    public:
        using Exception::Exception;
    };

    class NameException: public Exception {
        std::string exception_name() override { return "NameError"; }
    public:
        using Exception::Exception;
    };

    class RuntimeException: public Exception {
        std::string exception_name() override { return "RuntimeError"; }
    public:
        using Exception::Exception;
    };
}

#endif //ODO_PORT_EXCEPTION_H
