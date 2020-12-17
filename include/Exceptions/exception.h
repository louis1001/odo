//
// Created by Luis Gonzalez on 4/7/20.
//

#include <exception>
#include <sstream>
#include "Translations/lang.h"

#ifndef ODO_PORT_EXCEPTION_H
#define ODO_PORT_EXCEPTION_H
namespace Odo::Exceptions {
    class Exception: public std::exception {
    public:
        [[nodiscard]] virtual std::string msg() const noexcept {
            return ODO_ERROR_TXT;
        }
    };

    class OdoException: public Exception {
        unsigned int line_number{};
        unsigned int column_number{};

        std::string message{};

        [[nodiscard]] virtual std::string exception_name() const { return GEN_EXCP_NM; };
    public:
        explicit OdoException(const std::string& msg, unsigned int line_n=0, unsigned int col_n=0) {
            message = msg;
            line_number = line_n;
            column_number = col_n;
        }

        [[nodiscard]] std::string msg() const noexcept final {
            std::string ss;
            ss += exception_name() + " (" + MSG_LINE_TXT + " " + std::to_string(line_number) + ", " + MSG_COL_TXT + " " +
                    std::to_string(column_number) + ")\n\t" + message;
            return ss;
        }
        [[nodiscard]] virtual bool should_show_traceback() const { return true; }
    };

    class IOException: public Exception {
        std::string path;
    public:

        explicit IOException(const std::string& _path) {
            path = _path;
        }

        [[nodiscard]] std::string msg() const noexcept final {
            return std::string(IO_EXCP_MSG) + path + "'";
        };
    };

    class FileException: public OdoException {
        [[nodiscard]] std::string exception_name() const  override { return FILE_EXCP_NM; }
    public:
        using OdoException::OdoException;
    };

    class SyntaxException: public OdoException {
        [[nodiscard]] std::string exception_name() const  override { return SNTX_EXCP_NM; }

    public:
        using OdoException::OdoException;
        [[nodiscard]] bool should_show_traceback() const override { return false; }
    };

    class TypeException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return TP_EXCP_NM; }
    public:
        using OdoException::OdoException;
    };

    class ValueException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return VAL_EXCP_NM; }
    public:
        using OdoException::OdoException;
    };

    class NameException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return NM_EXCP_NM; }
    public:
        using OdoException::OdoException;
    };

    class FunctionCallException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return FNC_EXCP_NM; }
    public:
        using OdoException::OdoException;
    };

    class RecursionException: public OdoException {
        [[nodiscard]] std::string exception_name() const override { return REC_ECXP_NM; }
    public:
        using OdoException::OdoException;
    };
}

#endif //ODO_PORT_EXCEPTION_H
