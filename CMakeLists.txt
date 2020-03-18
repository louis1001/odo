cmake_minimum_required(VERSION 3.15)
project(odo_port)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(include/Lexer)

add_executable(odo_port
        include/Lexer/lexer.hpp
        include/Lexer/token.hpp
        include/main.hpp
        src/Lexer/lexer.cpp
        src/Lexer/token.cpp
        src/main.cpp
        src/Parser/parser.cpp include/Parser/parser.h src/Parser/AST.cpp include/Parser/AST.h)
