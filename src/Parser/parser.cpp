//
// Created by Luis Gonzalez on 3/17/20.
//

#include <utility>
#include <vector>
#include <algorithm>

#include "Parser/parser.h"
#include "utils.h"

Parser::Parser(Lexer lexer) : lexer(std::move(lexer)) {}

void Parser::eat(TokenType tp) {
    if (current_token.tp == tp) {
        current_token = lexer.getNextToken();
    } else {
        // TODO: Handle exception
        throw 1;
    }
}

void Parser::set_text(std::string t) {
    lexer.text = std::move(t);
    lexer.reset();

    current_token = lexer.getNextToken();
}

void Parser::ignore_nl() {
    while (current_token.tp == NL) {
        eat(NL);
    }
}

void Parser::statement_terminator() {
    auto block_terminator = {
        EOFT,
        RCUR,
        RPAR
    };

    if (current_token.tp == SEMI) {
        eat(SEMI);
        while (current_token.tp == NL) {
            eat(NL);
        }
    } else if (!contains_type(block_terminator, current_token.tp)){
        int cont = 0;
        do {
            eat(NL);
            cont++;
        } while (current_token.tp == NL);
    }
}

AST Parser::program() {
    auto result = block();

    if (current_token.tp != EOFT) {
        // TODO: Handle exceptions
        throw 1;
    }

    return result;
}

AST Parser::block() {
    return { Block, .lst_AST=statement_list() };
}

AST Parser::func_body(){
    return { FuncBody, .lst_AST=statement_list() };
}

AST Parser::class_body(){
    return { ClassBody, .lst_AST=statement_list() };
}

std::vector<AST> Parser::statement_list() {
    auto result = std::vector<AST>();

    ignore_nl();

    if(current_token.tp == RCUR) {
        return result;
    }

    while (current_token.tp != EOFT && current_token.tp != RCUR) {
        result.push_back(statement());
    }

    return result;
}

AST Parser::statement(bool with_term) {
    ignore_nl();
    AST ex;

    switch (current_token.tp) {
        case LCUR:
            eat(LCUR);
            ex = block();
            eat(RCUR);
            break;
        case IF:
            eat(IF);
            ex = ifstatement();
            break;
        case CLASS:
            eat(CLASS);
            ex = classstatement();
            break;
        case INIT:
            eat(INIT);
            ex = constructor();
            break;
        case FUNC:
            eat(FUNC);
            ex = function();
            break;
        case RET:
        {
            eat(RET);
            auto val = ternary_op();
            ex = {
                Return,
                .nodes={
                    {"val", val}
                }
            };
            break;
        }
        case FOR:
            eat(FOR);
            ex = forstatement();
            break;
        case WHILE:
            eat(WHILE);
            ex = whilestatement();
            break;
        case LOOP:
            eat(LOOP);
            ex = loopstatement();
            break;
        case BREAK:
            eat(BREAK);
            ex = {Break};
            break;
        case STATIC:
            eat(STATIC);
            ex = {StaticStatement, .nodes={
                    {
                        "statement",
                        statement(false)
                    }
                }
            };
            break;
        case EOFT:
        case RCUR:
        case RPAR:
            return AST { NoOp };
        case DEBUG:
            eat(DEBUG);
            return AST{Debug};
        default:
            ex = ternary_op();
            break;
    }

    if (with_term) {
        statement_terminator();
    }

    return ex;
}

AST Parser::classstatement() {
    auto name = current_token;

    eat(ID);

    Token inherits(NOTHING, "");

    if (current_token.tp == COLON) {
        eat(COLON);
        inherits = current_token;
        eat(ID);
    }

    eat(LCUR);
    auto body = class_body();
    eat(RCUR);

    return {
        Class,
        .token=name,
        .type=inherits,
        .nodes={
            {"body", body}
        }
    };
}

AST Parser::newInitializer() {
    auto name = current_token;
    eat(ID);

    std::vector<AST> argList;
    if (current_token.tp == LPAR) {
        eat(LPAR);

        while (current_token.tp != RPAR) {
            argList.push_back(ternary_op());

            if (current_token.tp != RPAR) {
                eat(COMMA);
            }
        }

        eat(RPAR);
    }

    return {
            ClassInitializer,
            .token=name,
            .lst_AST=argList
    };
}

AST Parser::constructor() {
    eat(LPAR);
    auto params = parameters();
    eat(RPAR);

    eat(LCUR);
    auto bl = func_body();
    eat(RCUR);

    return {
        ConstructorDecl,
        .lst_AST=params,
        .nodes={
            {"body", bl}
        }
    };
}

AST Parser::loopstatement() {
    AST body;

    if (current_token.tp == LCUR) {
        eat(LCUR);
        body = block();
        eat(RCUR);
    } else {
        body = statement();
    }

    return {
        Loop,
        .nodes={{"body", body}}
    };
}

AST Parser::whilestatement() {
    auto comp = ternary_op();

    AST body;
    if (current_token.tp == LCUR) {
        eat(LCUR);
        body = block();
        eat(RCUR);
    } else {
        body = statement();
    }

    return {
            While,
            .nodes={
                {"body", body},
                {"cond", comp}
            }
    };
}

AST Parser::forstatement() {
    eat(LPAR);
    auto initializer = statement();

    auto compar = ternary_op();
    statement_terminator();
    auto inc = statement();

    eat(RPAR);

    AST body;

    if (current_token.tp == LCUR) {
        eat(LCUR);
        body = block();
        eat(RCUR);
    } else {
        body = statement();
    }

    return {
        For,
        .nodes={
            {"ini", initializer},
            {"cond", compar},
            {"incr", inc},
            {"body", body},
        }
    };
}

AST Parser::ifstatement() {
    auto exp = ternary_op();

    if (current_token.tp == LCUR) {
        eat(LCUR);

        auto trueBlock = block();
        eat(RCUR);

        if (current_token.tp == ELSE) {
            eat(ELSE);

            AST elseBlock;

            if (current_token.tp == LCUR) {
                eat(LCUR);
                elseBlock = block();
                eat(RCUR);
            } else {
                eat(IF);
                elseBlock = ifstatement();
            }

            return {
                If,
                .nodes={
                    {"expr", exp},
                    {"trueb", trueBlock},
                    {"falseb", elseBlock}
                }
            };
        }

        return {
            If,
            .nodes={
                {"expr", exp},
                {"trueb", trueBlock},
                {"falseb", {NoOp}}
            }
        };
    } else {
        auto trueSta = statement();

        return {
            If,
            .nodes={
                {"expr", exp},
                {"trueb", trueSta},
                {"falseb", {NoOp}}
            }
        };
    }
}

AST Parser::function() {
    auto name = current_token;
    eat(ID);

    eat(LPAR);
    auto params = parameters();
    eat(RPAR);

    Token retType(NOTHING, "");
    if (current_token.tp == COLON) {
        eat(COLON);
        retType = current_token;

        eat(ID);
    }

    eat(LCUR);
    auto bl = func_body();
    eat(RCUR);

    return {
        FuncDecl,
        .token=name,
        .lst_AST=params,
        .type=retType,
        .nodes={{"body", bl}}
    };
}

std::vector<AST> Parser::parameters() {
    std::vector<AST> params;

    if (current_token.tp != RPAR) {
        auto typeToken = current_token;
        eat(ID);
        params.push_back(declaration(typeToken));

        while (current_token.tp != RPAR) {
            eat(COMMA);
            auto typeToken2 = current_token;
            eat(ID);
            params.push_back(declaration(typeToken2));
        }
    }

    return params;
}

AST Parser::declaration(const Token& token) {
    auto varName = current_token;
    eat(ID);

    AST assignment = {NoOp};

    if (current_token.tp == LBRA) {
        auto tp = token;
        eat(LBRA);
        eat(RBRA);

        while (current_token.tp == LBRA) {
            eat(LBRA);
            tp = Token(ID, tp.value + "[]");
            eat(RBRA);
        }

        if (current_token.tp == ASS) {
            eat(ASS);
            assignment = ternary_op();
        }

        return {
            ListDeclaration,
            .type=token,
            .token=varName,
            .nodes={{"initial", assignment}}
        };
    }

    if (current_token.tp == ASS) {
        eat(ASS);
        assignment = ternary_op();
    }

    return {
            Declaration,
            .type=token,
            .token=varName,
            .nodes={{"initial", assignment}}
    };
}

AST Parser::ternary_op() {
    ignore_nl();

    auto comp = or_comparison();

    while (current_token.tp == QUEST) {
        eat(QUEST);
        auto trueSection = or_comparison();

        eat(COLON);
        auto falseSection = or_comparison();

        comp = {
            TernaryOp,
            .nodes= {
                { "cond", comp },
                { "trueb", trueSection },
                { "falseb", falseSection }
            }
        };
    }

    return comp;
}

AST Parser::or_comparison() {
    auto node = and_comparison();

    while (current_token.tp == OR) {
        auto c_token = current_token;

        eat(OR);
        node = {
            BinOp,
            .token=c_token,
            .nodes={
                {"left", node},
                {"right", and_comparison()}
            }
        };
    }

    return node;
}

AST Parser::and_comparison() {
    auto node = comparison();

    while (current_token.tp == AND) {
        auto c_token = current_token;
        eat(AND);
        node = {
            BinOp,
            .token=c_token,
            .nodes={
                {"left", node},
                {"right", comparison()}
            }
        };
    }
    return node;
}

AST Parser::comparison() {
    auto exp = expression();
    auto compType = {EQU, NEQ, GT, LT, LET, GET};

    while (contains_type(compType, current_token.tp)) {
        auto c_token = current_token;
        eat(current_token.tp);
        exp = {
            BinOp,
            .token=c_token,
            .nodes={
                {"left", exp},
                {"right", expression()}
            }
        };
    }

    return exp;
}

AST Parser::expression() {
    auto node = term();

    auto exprType = {PLUS, MINUS};

    while (contains_type(exprType, current_token.tp)) {
        auto c_token = current_token;

        eat(current_token.tp);
        node = {
            BinOp,
            .token=c_token,
            .nodes={
                {"left", node},
                {"right", term()}
            }
        };
    }

    return node;
}

AST Parser::term() {
    auto node = prod();

    auto termType = {MUL, DIV};

    while (contains_type(termType, current_token.tp)) {
        auto c_token = current_token;

        eat(current_token.tp);
        node = {
            BinOp,
            .token=c_token,
            .nodes={
                {"left", node},
                {"right", prod()}
            }
        };
    }

    return node;
}

AST Parser::prod() {
    auto node = postfix();

    while (current_token.tp == POW) {
        auto c_token = current_token;

        eat(POW);
        node = {
            BinOp,
            .token=c_token,
            .nodes={
                {"left", node},
                {"right", postfix()}
            }
        };
    }

    return node;
}

AST Parser::postfix() {
    auto node = factor();

    auto postFixOp = {
            PLUSP, MINUSP,
            PLUSE, MINUSE, MULE, DIVE,
            DOT, DCOLON,
            LPAR,
            LBRA,
            ASS
    };

    while (contains_type(postFixOp, current_token.tp)) {
        switch (current_token.tp) {
            case PLUSP:
            case MINUSP:
            {
                Token binOP {MINUS, "-"};
                if (current_token.tp == PLUSP) {
                    binOP = {PLUS, "+"};
                    eat(PLUSP);
                } else {
                    eat(MINUSP);
                }
                auto one = Token(INT, "1");
                AST operation = {
                        BinOp,
                        .token=binOP,
                        .nodes={
                                {"left", node},
                                {"right", {
                                                 Int,
                                                 .token=one
                                         }},
                        }};

                node = {
                        Assignment,
                        .nodes={
                                {"right", operation}
                        }
                };
                break;
            }
            case PLUSE:
            case MINUSE:
            case MULE:
            case DIVE:
            {
                Token binOP = Token(MUL, "*");
                switch (current_token.tp) {
                    case PLUSE:
                        binOP = Token(PLUS, "+");
                    case MINUSE:
                        binOP = Token(MINUS, "-");
                    case MULE:
                        binOP = Token(MUL, "*");
                    default:
                        break;
                }
                eat(current_token.tp);

                AST operation = {
                    BinOp,
                    .token=binOP,
                    .nodes={
                        {"left", node},
                        {"right", expression()}
                    }
                };

                node = {
                    Assignment,
                    .nodes={
                        {"token", node},
                        {"right", operation}
                    }
                };

                break;
            }
            case DOT:
            {
                eat(DOT);
                auto member = current_token;
                eat(ID);
                node = {
                        MemberVar,
                        .token=member,
                        .nodes={
                                {"inst", node},
                        }
                };
                break;
            }
            case DCOLON:
            {
                eat(DCOLON);
                auto member = current_token;
                eat(ID);
                node = {
                    StaticVar,
                    .token=member,
                    .nodes={{"inst", node}}
                };
                break;
            }
            case LPAR:
            {
                eat(LPAR);

                std::vector<AST> argList;

                while (current_token.tp != RPAR) {
                    argList.push_back(ternary_op());

                    ignore_nl();
                    if (current_token.tp != RPAR) {
                        eat(COMMA);
                    }
                    ignore_nl();
                }

                eat(RPAR);
                Token name(NOTHING, "");
                if (node.tp == Variable){
                    name = node.token;
                }

                node = {
                    FuncCall,
                    .nodes = {{"fun", node}},
                    .token=name,
                    .lst_AST=argList
                };
                break;
            }
            case LBRA:
                eat(LBRA);
                node = {
                    Index,
                    .nodes = {
                        {"val", node},
                        {"expr", ternary_op()},
                    }
                };
                eat(RBRA);
                break;
            case ASS:
                eat(ASS);
                node = {
                    Assignment,
                    .nodes = {
                        {"token", node},
                        {"right", ternary_op()}
                    }
                };
                break;
            default:
                break;
        }
    }

    return node;
}

AST Parser::funcexpression() {
    eat(LPAR);
    auto params = parameters();

    eat(RPAR);

    eat(ARROW);

    AST body;
    if (current_token.tp == LCUR) {
        eat(LCUR);
        body = func_body();
        eat(RCUR);
    } else {
        body = ternary_op();
    }

    return {
        FuncExpression,
        .lst_AST=params,
        .type=Token(NOTHING, ""),
        .nodes={
            {"body", body}
        }
    };
}

AST Parser::factor() {
    ignore_nl();
    switch (current_token.tp) {
        case REAL:
        {
            auto c = current_token;
            eat(REAL);
            return {
                Double,
                .token=c
            };
        }
        case INT:
        {
            auto c = current_token;
            eat(INT);
            return {
                    Int,
                    .token=c
            };
        }
        case STR:
        {
            auto c = current_token;
            eat(STR);
            return {
                    Str,
                    .token=c
            };
        }
        case BOOL:
        {
            auto c = current_token;
            eat(BOOL);
            return {
                Bool,
                .token=c
            };
        }
        case NOT:
        {
            eat(NOT);
            auto c = ternary_op();
            AST notComp = {
                BinOp,
                .token=Token(EQU, "=="),
                .nodes={
                    {"left", c},
                    {"right", AST {
                            Bool,
                            .token=Token(BOOL, "false")
                    }}
                }
            };

            return notComp;
        }
        case LPAR:
        {
            eat(LPAR);
            auto exp = ternary_op();
            eat(RPAR);
            return exp;
        }
        case PLUS:
        case MINUS:
        {
            auto op = current_token;
            eat(current_token.tp);
            return {
                UnaryOp,
                .token=op,
                .nodes={{ "right", factor() }}
            };
        }
        case NEW:
            eat(NEW);
            return newInitializer();
        case ID:
        {
            auto idToken = current_token;
            eat(ID);

            if (current_token.tp == ID) {
                return declaration(idToken);
            }

            return {
                    Variable,
                    .token=idToken
            };
        }
        case LBRA:
        {
            eat(LBRA);
            std::vector<AST> contents;
            if (current_token.tp != RBRA) {
                contents.push_back(ternary_op());
                while (current_token.tp != RBRA) {
                    eat(COMMA);
                    if (current_token.tp != RBRA) {
                        contents.push_back(ternary_op());
                        ignore_nl();
                    }
                }
            }
            eat(RBRA);
            return {
                    ListExpression,
                    .lst_AST=contents
            };
        }
        case NULLT:
            eat(NULLT);
            return { Null };
        case FUNC:
            eat(FUNC);
            return funcexpression();
        default:
            break;
    }

    return AST { NoOp };
}

Parser::Parser(): lexer(Lexer()) {}

