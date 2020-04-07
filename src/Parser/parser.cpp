//
// Created by Luis Gonzalez on 3/17/20.
//

#include <utility>
#include <vector>
#include <algorithm>

#include "Parser/parser.h"
#include "utils.h"

namespace Odo::Parsing{
    using namespace Lexing;
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
        return add_dbg_info({ Block, .lst_AST=statement_list() });
    }

    AST Parser::func_body(){
        return add_dbg_info({ FuncBody, .lst_AST=statement_list() });
    }

    AST Parser::class_body(){
        return add_dbg_info({ ClassBody, .lst_AST=statement_list() });
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
                ex = add_dbg_info({
                        Return
                });
                auto val = ternary_op();
                ex.nodes["val"] = val;
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
            case CONTINUE:
                eat(CONTINUE);
                ex = {Continue};
                break;
            case STATIC:
                eat(STATIC);
                ex = add_dbg_info({StaticStatement});
                ex.nodes["statement"] = statement(false);
                break;
            case EOFT:
            case RCUR:
            case RPAR:
                return add_dbg_info({ NoOp });
            case DEBUG:
                ex = add_dbg_info({Debug});
                eat(DEBUG);
                return ex;
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
        AST result = add_dbg_info({Class});
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

        result.token = name;
        result.type = inherits;
        result.nodes["body"] = body;

        return result;
    }

    AST Parser::newInitializer() {
        AST result = add_dbg_info({ClassInitializer});
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

        result.token = name;
        result.lst_AST = argList;

        return result;
    }

    AST Parser::constructor() {
        AST result = add_dbg_info({ConstructorDecl});
        eat(LPAR);
        auto params = parameters();
        eat(RPAR);

        eat(LCUR);
        auto bl = func_body();
        eat(RCUR);

        result.lst_AST = params;
        result.nodes["body"] = bl;
        return result;
    }

    AST Parser::loopstatement() {
        AST result = add_dbg_info({ Loop });
        AST body;

        if (current_token.tp == LCUR) {
            eat(LCUR);
            body = block();
            eat(RCUR);
        } else {
            body = statement();
        }
        result.nodes["body"] = body;

        return result;
    }

    AST Parser::whilestatement() {
        AST result = add_dbg_info({While});
        auto comp = ternary_op();

        AST body;
        if (current_token.tp == LCUR) {
            eat(LCUR);
            body = block();
            eat(RCUR);
        } else {
            body = statement();
        }
        result.nodes = {
                {"body", body},
                {"cond", comp}
        };

        return result;
    }

    AST Parser::forstatement() {
        AST result = add_dbg_info({For});
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

        result.nodes = {
                {"ini", initializer},
                {"cond", compar},
                {"incr", inc},
                {"body", body},
        };

        return result;
    }

    AST Parser::ifstatement() {
        AST result = add_dbg_info({If});
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

                result.nodes = {
                        {"expr", exp},
                        {"trueb", trueBlock},
                        {"falseb", elseBlock}
                };

                return result;
            }

            result.nodes = {
                    {"expr", exp},
                    {"trueb", trueBlock},
                    {"falseb", {}}
            };

            return result;
        } else {

            result.nodes = {
                    {"expr", exp},
                    {"trueb", statement()},
                    {"falseb", {}}
            };

            return result;
        }
    }

    AST Parser::function() {
        AST result = add_dbg_info({FuncDecl});
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

        result.token = name;
        result.lst_AST = params;
        result.type = retType;
        result.nodes = {{"body", bl}};

        return result;
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
        AST result = add_dbg_info({});
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

            result.tp = ListDeclaration;
            result.type=token;
            result.token=varName;
            result.nodes = {{"initial", assignment}};

            return result;
        }

        if (current_token.tp == ASS) {
            eat(ASS);
            assignment = ternary_op();
        }

        result.tp = Declaration;
        result.type=token;
        result.token=varName;
        result.nodes = {{"initial", assignment}};

        return result;
    }

    AST Parser::ternary_op() {
        ignore_nl();

        auto comp = or_comparison();

        while (current_token.tp == QUEST) {
            AST result =add_dbg_info({TernaryOp});
            eat(QUEST);
            auto trueSection = or_comparison();

            eat(COLON);
            auto falseSection = or_comparison();

            result.nodes = {
                { "cond", comp },
                { "trueb", trueSection },
                { "falseb", falseSection }
            };
            comp = result;
        }

        return comp;
    }

    AST Parser::or_comparison() {
        auto node = and_comparison();

        while (current_token.tp == OR) {
            AST result = add_dbg_info({BinOp, .token=current_token});

            eat(OR);
            result.nodes = {
                {"left", node},
                {"right", and_comparison()}
            };
            node = result;
        }

        return node;
    }

    AST Parser::and_comparison() {
        auto node = comparison();

        while (current_token.tp == AND) {
            AST result = add_dbg_info({BinOp, .token=current_token});
            eat(AND);

            result.nodes = {
                {"left", node},
                {"right", comparison()}
            };
            node = result;
        }
        return node;
    }

    AST Parser::comparison() {
        auto exp = expression();
        auto compType = {EQU, NEQ, GT, LT, LET, GET};

        while (contains_type(compType, current_token.tp)) {
            auto c_token = current_token;
            AST result = add_dbg_info({BinOp, .token=c_token});
            eat(current_token.tp);
            result.nodes = {
                {"left", exp},
                {"right", expression()}
            };
            exp = result;
        }

        return exp;
    }

    AST Parser::expression() {
        auto node = term();

        auto exprType = {PLUS, MINUS};

        while (contains_type(exprType, current_token.tp)) {
            auto c_token = current_token;
            AST result = add_dbg_info({BinOp, .token=c_token});

            eat(current_token.tp);

            result.nodes = {
                {"left", node},
                {"right", term()}
            };
            node = result;
        }

        return node;
    }

    AST Parser::term() {
        auto node = prod();

        auto termType = {MUL, DIV, MOD};

        while (contains_type(termType, current_token.tp)) {
            auto c_token = current_token;
            AST result = add_dbg_info({BinOp, .token=c_token});

            eat(current_token.tp);

            result.nodes = {
                    {"left", node},
                    {"right", prod()}
            };
            node = result;
        }

        return node;
    }

    AST Parser::prod() {
        auto node = postfix();

        while (current_token.tp == POW) {
            auto c_token = current_token;
            AST result = add_dbg_info({BinOp, .token=c_token});

            eat(POW);
            result.nodes = {
                {"left", node},
                {"right", postfix()}
            };
            node = result;
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
            AST result = add_dbg_info({});
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
                        },
                        .line_number=result.line_number,
                        .column_number=result.column_number
                    };

                    result.tp = Assignment,
                    result.nodes={
                        {"token", node},
                        {"right", operation}
                    };

                    node = result;
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
                            break;
                        case MINUSE:
                            binOP = Token(MINUS, "-");
                            break;
                        case MULE:
                            binOP = Token(MUL, "*");
                            break;
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
                        },
                        .line_number = result.line_number,
                        .column_number = result.column_number
                    };

                    result.tp = Assignment;
                    result.nodes = {
                        {"token", node},
                        {"right", operation}
                    };

                    node = result;
                    break;
                }
                case DOT:
                {
                    result.tp = MemberVar;
                    eat(DOT);
                    result.token = current_token;
                    result.nodes = {
                        {"inst", node},
                    };
                    eat(ID);
                    node = result;
                    break;
                }
                case DCOLON:
                {
                    result.tp = StaticVar;
                    eat(DCOLON);
                    result.token = current_token;
                    eat(ID);
                    result.nodes = {{"inst", node}};
                    node = result;
                    break;
                }
                case LPAR:
                {
                    result.tp = FuncCall;
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
                    result.lst_AST = argList;

                    eat(RPAR);
                    Token name(NOTHING, "");
                    if (node.tp == Variable){
                        name = node.token;
                    }

                    result.token = name;
                    result.nodes = {{"fun", node}};

                    node = result;
                    break;
                }
                case LBRA:
                    eat(LBRA);
                    result.tp = Index;
                    result.nodes = {
                        {"val", node},
                        {"expr", ternary_op()},
                    };
                    node = result;
                    eat(RBRA);
                    break;
                case ASS:
                    result.tp = Assignment;
                    result.nodes = {
                        {"token", node},
                    };
                    eat(ASS);
                    result.nodes["right"] = ternary_op();
                    node = result;
                    break;
                default:
                    break;
            }
        }

        return node;
    }

    AST Parser::funcexpression() {
        AST result = add_dbg_info({FuncExpression});
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

        result.lst_AST = params;
        result.type = Token(NOTHING, "");
        result.nodes = {{"body", body}};

        return result;
    }

    AST Parser::factor() {
        ignore_nl();
        switch (current_token.tp) {
            case REAL:
            {
                auto double_literal = add_dbg_info({
                    Double,
                    .token=current_token
               });
                eat(REAL);
                return double_literal;
            }
            case INT:
            {
                auto int_literal = add_dbg_info({
                    Int,
                    .token=current_token
                });
                eat(INT);
                return int_literal;
            }
            case STR:
            {
                auto str_literal = add_dbg_info({
                    Str,
                    .token=current_token
                });
                eat(STR);
                return str_literal;
            }
            case BOOL:
            {
                auto bool_literal = add_dbg_info({
                    Bool,
                    .token=current_token
                });
                eat(BOOL);
                return bool_literal;
            }
            case NOT:
            {
                eat(NOT);
                auto result = add_dbg_info({
                    BinOp,
                    .token=Token(EQU, "=="),
                    .nodes={
                            {"right", AST {
                                Bool,
                                .token=Token(BOOL, "false")
                            }}
                    }
                });
                auto c = ternary_op();
                result.nodes["left"] = c;

                return result;
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
                auto result = add_dbg_info({
                    UnaryOp,
                    .token=current_token,
                });
                eat(current_token.tp);
                result.nodes["right"] = factor();
                return result;
            }
            case NEW:
                eat(NEW);
                return newInitializer();
            case ID:
            {
                auto idToken = current_token;
                auto result = add_dbg_info({Variable, .token=idToken});
                eat(ID);

                if (current_token.tp == ID) {
                    return declaration(idToken);
                }

                return result;
            }
            case LBRA:
            {
                auto list_expr = add_dbg_info({ListExpression});
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
                list_expr.lst_AST = contents;
                eat(RBRA);
                return list_expr;
            }
            case NULLT:
            {
                auto null_ret = add_dbg_info({Null});
                eat(NULLT);
                return null_ret;
            }
            case FUNC:
                eat(FUNC);
                return funcexpression();
            default:
                break;
        }

        return add_dbg_info({ NoOp });
    }

    Parser::Parser(): lexer(Lexer()) {}

    std::vector<AST> Parser::program_content() {
        return statement_list();
    }

    AST Parser::add_dbg_info(AST ins) {
        ins.line_number = lexer.getCurrentLine();
        ins.column_number = lexer.getCurrentCol();
        return std::move(ins);
    }
}
