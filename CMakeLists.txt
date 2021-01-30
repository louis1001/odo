cmake_minimum_required(VERSION 3.10)
project(odo)

set(CMAKE_CXX_STANDARD 20)

if (UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
else()
    string(REGEX REPLACE "/W[3|4]" "/W4" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -WX")
endif()
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
        src/IO/io.cpp
        include/Interpreter/frame.h
        include/Parser/AST/Node.h
        src/Parser/AST/Node.cpp

        include/Parser/AST/DoubleNode.h
        include/Parser/AST/IntNode.h
        include/Parser/AST/BoolNode.h
        include/Parser/AST/StrNode.h
        include/Parser/AST/TernaryOpNode.h
        include/Parser/AST/BinOpNode.h
        include/Parser/AST/UnaryOpNode.h
        include/Parser/AST/NoOpNode.h
        include/Parser/AST/VarDeclarationNode.h
        include/Parser/AST/ListDeclarationNode.h
        include/Parser/AST/VariableNode.h
        include/Parser/AST/AssignmentNode.h
        include/Parser/AST/ListExpressionNode.h
        include/Parser/AST/BlockNode.h
        include/Parser/AST/FuncExpressionNode.h
        include/Parser/AST/FuncDeclNode.h
        include/Parser/AST/FuncCallNode.h
        include/Parser/AST/FuncBodyNode.h
        include/Parser/AST/ReturnNode.h
        include/Parser/AST/IfNode.h
        include/Parser/AST/ForNode.h
        include/Parser/AST/ForEachNode.h
        include/Parser/AST/FoRangeNode.h
        include/Parser/AST/WhileNode.h
        include/Parser/AST/LoopNode.h
        include/Parser/AST/BreakNode.h
        include/Parser/AST/ContinueNode.h
        include/Parser/AST/NullNode.h
        include/Parser/AST/DebugNode.h
        include/Parser/AST/ModuleNode.h
        include/Parser/AST/ImportNode.h
        include/Parser/AST/DefineNode.h
        include/Parser/AST/EnumNode.h
        include/Parser/AST/ClassNode.h
        include/Parser/AST/ClassBodyNode.h
        include/Parser/AST/InstanceBodyNode.h
        include/Parser/AST/ClassInitializerNode.h
        include/Parser/AST/ConstructorDeclNode.h
        include/Parser/AST/ConstructorCallNode.h
        include/Parser/AST/StaticStatementNode.h
        include/Parser/AST/MemberVarNode.h
        include/Parser/AST/StaticVarNode.h
        include/Parser/AST/IndexNode.h

        src/Parser/AST/DoubleNode.cpp
        src/Parser/AST/IntNode.cpp
        src/Parser/AST/BoolNode.cpp
        src/Parser/AST/StrNode.cpp
        src/Parser/AST/TernaryOpNode.cpp
        src/Parser/AST/BinOpNode.cpp
        src/Parser/AST/UnaryOpNode.cpp
        src/Parser/AST/VarDeclarationNode.cpp
        src/Parser/AST/ListDeclarationNode.cpp
        src/Parser/AST/VariableNode.cpp
        src/Parser/AST/AssignmentNode.cpp
        src/Parser/AST/ListExpressionNode.cpp
        src/Parser/AST/BlockNode.cpp
        src/Parser/AST/FuncExpressionNode.cpp
        src/Parser/AST/FuncDeclNode.cpp
        src/Parser/AST/FuncCallNode.cpp
        src/Parser/AST/FuncBodyNode.cpp
        src/Parser/AST/ReturnNode.cpp
        src/Parser/AST/IfNode.cpp
        src/Parser/AST/ForNode.cpp
        src/Parser/AST/ForEachNode.cpp
        src/Parser/AST/FoRangeNode.cpp
        src/Parser/AST/WhileNode.cpp
        src/Parser/AST/LoopNode.cpp
        src/Parser/AST/ModuleNode.cpp
        src/Parser/AST/ImportNode.cpp
        src/Parser/AST/DefineNode.cpp
        src/Parser/AST/EnumNode.cpp
        src/Parser/AST/ClassNode.cpp
        src/Parser/AST/ClassBodyNode.cpp
        src/Parser/AST/InstanceBodyNode.cpp
        src/Parser/AST/ClassInitializerNode.cpp
        src/Parser/AST/ConstructorDeclNode.cpp
        src/Parser/AST/ConstructorCallNode.cpp
        src/Parser/AST/StaticStatementNode.cpp
        src/Parser/AST/MemberVarNode.cpp
        src/Parser/AST/StaticVarNode.cpp
        src/Parser/AST/IndexNode.cpp

        include/Parser/AST/Forward.h
        include/Translations/lexer_en.h
        include/Translations/lexer_es.h
        include/Translations/lang.h
        include/Translations/parser_en.h
        include/Translations/parser_es.h
        include/Translations/exception_en.h
        include/Translations/exception_es.h
        include/Translations/Interpreter_en.h
        include/Translations/Interpreter_es.h
        include/Translations/value_en.h
        include/Translations/main_en.h
        include/Translations/value_es.h
        include/Translations/main_es.h
        include/Translations/symbol_en.h
        include/Translations/symbol_es.h
        include/SemAnalyzer/SemanticAnalyzer.h
        include/Translations/SemanticAnalyzer_en.h
        include/Translations/SemanticAnalyzer_es.h
        include/Translations/Modules_en.h
        include/Translations/Modules_es.h

        include/SemAnalyzer/NodeResult.h
        src/SemAnalyzer/SemanticAnalyzer.cpp
        src/Modules/NativeModule.cpp
        include/Modules/NativeModule.h
        include/Modules/IOModule.h
        src/Modules/IOModule.cpp
        include/Modules/MathModule.h
        include/Modules/TermModule.h
        include/Modules/TermColorsModule.h)

target_compile_definitions(odo PUBLIC LANG_USE_ES=0)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(odo PUBLIC DEBUG_MODE=1)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_definitions(odo PUBLIC DEBUG_MODE=0)
    add_custom_command(TARGET odo
        POST_BUILD
        COMMAND cd "../tests"
        COMMAND python odo_tests.py
    )
endif()