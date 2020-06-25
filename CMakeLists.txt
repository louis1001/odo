cmake_minimum_required(VERSION 3.10)
project(odo)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -Wall")

include_directories(include)
include_directories(include/Lexer)

add_executable(odo
        include/Lexer/lexer.hpp
        include/Lexer/token.hpp
        include/main.hpp
        src/Lexer/lexer.cpp
        src/Lexer/token.cpp
        src/main.cpp
        src/Parser/parser.cpp
        include/Parser/parser.h
        src/Parser/AST.cpp
        include/Parser/AST.h
        src/Interpreter/Interpreter.cpp
        include/Interpreter/Interpreter.h
        src/Interpreter/value.cpp
        include/Interpreter/value.h
        src/Interpreter/symbol.cpp
        include/Interpreter/symbol.h
        include/utils.h
        src/utils.cpp
        src/Exceptions/exception.cpp
        include/Exceptions/exception.h
        include/IO/io.h
        src/IO/io.cpp)
