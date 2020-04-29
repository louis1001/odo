//
// Created by Luis Gonzalez on 4/7/20.
//

#include <exception>
#include <sstream>

#ifndef ODO_PORT_EXCEPTION_H
#define ODO_PORT_EXCEPTION_H
namespace Odo::Exceptions {
    class Exception: std::exception {
    public:
        virtual const char* what() noexcept {
            return "OdoError";
        }
    };

    class OdoException: Exception {
        unsigned int line_number{};
        unsigned int column_number{};

        std::string message{};

        virtual std::string exception_name() { return "GenericError"; };
    public:
        explicit OdoException(const std::string& msg, unsigned int line_n=0, unsigned int col_n=0) {
            message = msg;
            line_number = line_n;
            column_number = col_n;
        }

        const char* what() noexcept final {
            std::string ss;
            ss += exception_name() + " (line " + std::to_string(line_number) + ", column " +
                    std::to_string(column_number) + ")\n\t" + message;
            return ss.c_str();
        }
    };

    class IOException: public Exception {
        std::string path;
    public:

        explicit IOException(const std::string& _path) {
            path = _path;
        }

        const char* what() noexcept final {
            return (std::string("IOError\n\tCannot open file '") + path + "'").c_str();
        };
    };

    class FileException: public OdoException {
        std::string exception_name() override { return "FileError"; }
    public:
        using OdoException::OdoException;
    };

    class SyntaxException: public OdoException {
        std::string exception_name() override { return "SyntaxError"; }
    public:
        using OdoException::OdoException;
    };

    class TypeException: public OdoException {
        std::string exception_name() override { return "TypeError"; }
    public:
        using OdoException::OdoException;
    };

    class ValueException: public OdoException {
        std::string exception_name() override { return "ValueError"; }
    public:
        using OdoException::OdoException;
    };

    class NameException: public OdoException {
        std::string exception_name() override { return "NameError"; }
    public:
        using OdoException::OdoException;
    };

    class RuntimeException: public OdoException {
        std::string exception_name() override { return "RuntimeError"; }
    public:
        using OdoException::OdoException;
    };
}

#endif //ODO_PORT_EXCEPTION_H
