//
// Created by Luis Gonzalez on 7/7/20.
//

#ifndef ODO_NODE_H
#define ODO_NODE_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "token.hpp"

namespace Odo::Parsing {
    enum class NodeType {
        Double,
        Int,
        Bool,
        Str,

        TernaryOp,
        BinOp,
        UnaryOp,
        NoOp,

        VarDeclaration,
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
        ForEach,
        FoRange,
        While,
        Loop,
        Break,
        Continue,
        Null,

        Debug,

        Module,
        Import,

        Define,

        Enum,

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

    struct Node {
        unsigned int line_number{};
        unsigned int column_number{};

        virtual NodeType kind()=0;
        template<typename T>
        static std::shared_ptr<T> as(const std::shared_ptr<Node>& n) {
            return std::dynamic_pointer_cast<T>(n);
        }
    };
}

#endif //ODO_NODE_H
