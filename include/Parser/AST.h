//
// Created by Luis Gonzalez on 3/17/20.
//

#ifndef ODO_PORT_AST_H
#define ODO_PORT_AST_H
#include <string>
#include <vector>
#include <map>

#include "token.hpp"

namespace Odo::Parsing {
    enum ASTType {
        Double,
        Int,
        Bool,
        Str,

        TernaryOp,
        BinOp,
        UnaryOp,
        NoOp,

        Declaration,
        ListDeclaration,
        Variable,
        Assignment,

        ListExpression,
        Block,
        FuncExpression,
        FuncDecl,
        FuncCall,
        FuncBody,
        Return,

        If,
        For,
        While,
        Loop,
        Break,
        Continue,
        Null,

        Debug,

        Class,
        ClassBody,
        InstanceBody,
        ClassInitializer,
        ConstructorDecl,
        ConstructorCall,
        StaticStatement,
        MemberVar,
        StaticVar,

        Index
    };

    struct AST {
        ASTType tp;

        Lexing::Token token = Lexing::Token(Lexing::NOTHING, "");
        Lexing::Token type = Lexing::Token(Lexing::NOTHING, "");

        std::map<std::string, AST> nodes;

        std::vector<AST> lst_AST;
    };
}
#endif //ODO_PORT_AST_H
