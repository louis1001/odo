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
        [[nodiscard]] virtual std::string msg() const noexcept {
            return "OdoError";
        }
    };

    class OdoException: Exception {
        unsigned int line_number{};
        unsigned int column_number{};

        std::string message{};

        [[nodiscard]] virtual std::string exception_name() const { return "GenericError"; };
    public:
        explicit OdoException(const std::string& msg, unsigned int line_n=0, unsigned int col_n=0) {
            message = msg;
            line_number = line_n;
            column_number = col_n;
        }

        [[nodiscard]] std::string msg() const noexcept final {
            std::string ss;
            ss += exception_name() + " (line " + std::to_string(line_number) + ", column " +
                    std::to_string(column_number) + ")\n\t" + message;
            return ss;
        }
        virtual bool should_show_traceback() const { return true; }
    };

    class IOException: public Exception {
        std::string path;
    public:

        explicit IOException(const std::string& _path) {
            path = _path;
        }

        [[nodiscard]] std::string msg() const noexcept final {
            return std::string("IOError\n\tCannot open file '") + path + "'";
        };
    };

    class FileException: public OdoException {
        [[nodiscard]] std::string exception_name() const  override { return "FileError"; }
    public:
        using OdoException::OdoException;
    };

    class SyntaxException: public OdoException {
        [[nodiscard]] std::string exception_name() const  override { return "SyntaxError"; }

    public:
        using OdoException::OdoException;
        bool should_show_traceback() const override { return false; }
    };

    class TypeException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return "TypeError"; }
    public:
        using OdoException::OdoException;
    };

    class ValueException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return "ValueError"; }
    public:
        using OdoException::OdoException;
    };

    class NameException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return "NameError"; }
    public:
        using OdoException::OdoException;
    };

    class FunctionCallException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return "FunctionCallError"; }
    public:
        using OdoException::OdoException;
    };

    class RecursionException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return "RecursionError"; }
    public:
        using OdoException::OdoException;
    };
}

#endif //ODO_PORT_EXCEPTION_H
