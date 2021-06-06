//
// Created by Luis Gonzalez on 3/17/20.
//

#include <utility>
#include <vector>
#include <algorithm>

#include "Parser/parser.h"
#include "utils.h"
#include "Exceptions/exception.h"

#include "Translations/lang.h"

#include "Parser/AST/DoubleNode.h"
#include "Parser/AST/IntNode.h"
#include "Parser/AST/BoolNode.h"
#include "Parser/AST/StrNode.h"
#include "Parser/AST/TernaryOpNode.h"
#include "Parser/AST/BinOpNode.h"
#include "Parser/AST/UnaryOpNode.h"
#include "Parser/AST/NoOpNode.h"
#include "Parser/AST/VarDeclarationNode.h"
#include "Parser/AST/ListDeclarationNode.h"
#include "Parser/AST/VariableNode.h"
#include "Parser/AST/AssignmentNode.h"
#include "Parser/AST/ListExpressionNode.h"
#include "Parser/AST/BlockNode.h"
#include "Parser/AST/FuncExpressionNode.h"
#include "Parser/AST/FuncDeclNode.h"
#include "Parser/AST/FuncCallNode.h"
#include "Parser/AST/FuncBodyNode.h"
#include "Parser/AST/ReturnNode.h"
#include "Parser/AST/IfNode.h"
#include "Parser/AST/ForNode.h"
#include "Parser/AST/ForEachNode.h"
#include "Parser/AST/FoRangeNode.h"
#include "Parser/AST/WhileNode.h"
#include "Parser/AST/LoopNode.h"
#include "Parser/AST/BreakNode.h"
#include "Parser/AST/ContinueNode.h"
#include "Parser/AST/NullNode.h"
#include "Parser/AST/DebugNode.h"
#include "Parser/AST/ModuleNode.h"
#include "Parser/AST/ImportNode.h"
#include "Parser/AST/DefineNode.h"
#include "Parser/AST/EnumNode.h"
#include "Parser/AST/ClassNode.h"
#include "Parser/AST/ClassBodyNode.h"
#include "Parser/AST/ClassInitializerNode.h"
#include "Parser/AST/ConstructorDeclNode.h"
#include "Parser/AST/StaticStatementNode.h"
#include "Parser/AST/MemberVarNode.h"
#include "Parser/AST/StaticVarNode.h"
#include "Parser/AST/IndexNode.h"

namespace Odo::Parsing{
    using namespace Lexing;

    void Parser::eat(TokenType tp) {
        if (current_token.tp == tp) {
            current_token = lexer.getNextToken();
        } else {
            std::string err_msg = UNEXPECTED_TK_EXCP;
            err_msg += current_token.value;
            err_msg += "'";
            throw Exceptions::SyntaxException(err_msg, lexer.getCurrentLine(), lexer.getCurrentCol());
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

    std::shared_ptr<Node> Parser::program() {
        auto result = block();
        eat(EOFT);

        return result;
    }

    std::shared_ptr<Node> Parser::block() {
        return add_dbg_info(BlockNode::create(statement_list()));
    }

    std::shared_ptr<Node> Parser::func_body(){
        return add_dbg_info(FuncBodyNode::create(statement_list()));
    }

    std::shared_ptr<Node> Parser::class_body(){
        return add_dbg_info(ClassBodyNode::create(statement_list()));
    }

    std::vector<std::shared_ptr<Node>> Parser::statement_list() {
        auto result = std::vector<std::shared_ptr<Node>>();

        ignore_nl();

        if(current_token.tp == RCUR) {
            return result;
        }

        while (current_token.tp != EOFT && current_token.tp != RCUR) {
            result.push_back(statement());
        }

        return result;
    }

    std::shared_ptr<Node> Parser::statement(bool with_term) {
        ignore_nl();
        std::shared_ptr<Node> ex;

        auto line_start = line();
        auto col_start = column();

        switch (current_token.tp) {
            case LCUR:
                eat(LCUR);
                ex = block();
                eat(RCUR);
                break;
            case VAR:
                eat(VAR);
                ex = declaration();
                break;
            case MODULE:
                eat(MODULE);
                ex = module_statement();
                break;
            case IMPORT:
            {
                eat(IMPORT);
                auto path = current_token;
                if (current_token.tp == STR){
                    eat(STR);
                } else {
                    eat(ID);
                }

                Token name{NOTHING, ""};

                if (current_token.tp == AS) {
                    eat(AS);
                    name = current_token;
                    eat(ID);
                }

                ex = ImportNode::create(path, name);
                break;
            }
            case DEFINE:
                eat(DEFINE);
                ex = define_statement();
                break;
            case IF:
                eat(IF);
                ex = ifstatement();
                break;
            case ENUM:
                eat(ENUM);
                ex = enumstatement();
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
                ex = ReturnNode::create(val);
                break;
            }
            case FOR:
                eat(FOR);
                ex = forstatement();
                break;
            case FOREACH:
                eat(FOREACH);
                ex = foreachstatement();
                break;
            case FORANGE:
                eat(FORANGE);
                ex = forangestatement();
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
                ex = BreakNode::create();
                break;
            case CONTINUE:
                eat(CONTINUE);
                ex = ContinueNode::create();
                break;
            case STATIC:
                eat(STATIC);
                ex = StaticStatementNode::create(statement(false));
                break;
            case EOFT:
            case RCUR:
            case RPAR:
                ex = NoOpNode::create();
                break;
            case DEBUG:
                ex = DebugNode::create();
                eat(DEBUG);
                break;
            default:
                ex = ternary_op();
                break;
        }

        if (with_term) {
            statement_terminator();
        }

        ex->line_number = line_start;
        ex->column_number = col_start;

        return ex;
    }

    std::shared_ptr<Node> Parser::module_statement() {
        auto ln = line();
        auto cl = column();
        auto name = current_token;

        eat(ID);

        eat(LCUR);
        auto body = statement_list();
        eat(RCUR);

        auto result = ModuleNode::create(name, body);
        result->line_number = ln;
        result->column_number = cl;
        return result;
    }

    std::shared_ptr<Node> Parser::get_type() {
        auto nd = VariableNode::create(current_token);
        eat(ID);
        // Don't mind the member variables.
        // Those should not be types any ways. Right?
        while (current_token.tp == DCOLON) {
            eat(DCOLON);
            nd = StaticVarNode::create(nd, current_token);
            eat(ID);
        }
        return nd;
    }

    std::shared_ptr<Node> Parser::get_full_type() {
        std::shared_ptr<Node> tp;

//        if (current_token.tp == LT) {
//            auto func_tp = get_arg_types();
//        }

        if (current_token.tp == ID) {
            tp = VariableNode::create(current_token);
            eat(ID);
        }

        while (current_token.tp == DCOLON) {
            eat(DCOLON);
            auto right_side = current_token;
            eat(ID);
            tp = StaticVarNode::create(tp, right_side);
        }

        while (current_token.tp == LBRA) {
            eat(LBRA);
            eat(RBRA);

            tp = IndexNode::create(tp, NoOpNode::create());
        }

        if (!tp) {
            // TODO: Translate this
            throw Exceptions::SyntaxException(
                    "Unexpected token. Expected a type for variable declaration",
                    line(),
                    column());
        }

        return tp;
    }

    Parser::function_type Parser::get_arg_types() {
        std::vector<std::pair<Token, bool>> arguments;
        Token retType{NOTHING, ""};

        eat(LT);
        while (current_token.tp == ID) {
            auto tp = current_token;
            eat(ID);
            while (current_token.tp == LBRA) {
                eat(LBRA);
                tp.value += "[]";
                eat(RBRA);
            }

            auto is_optional = false;
            if (current_token.tp == QUEST) {
                is_optional = true;
                eat(QUEST);
            }

            arguments.push_back({tp, is_optional});
            if (current_token.tp != COMMA) break;
            eat(COMMA);
        }

        if(current_token.tp == COLON) {
            eat(COLON);
            retType = current_token;
            eat(ID);
            while (current_token.tp == LBRA) {
                eat(LBRA);
                retType.value += "[]";
                eat(RBRA);
            }
        }
        eat(GT);

        return {arguments, retType};
    }

    std::shared_ptr<Node> Parser::define_statement() {
        auto func_type = get_arg_types();

        eat(AS);

        auto name = current_token;
        eat(ID);

        return DefineNode::create(func_type.args, func_type.retType, name);
    }

    std::shared_ptr<Node> Parser::enumstatement() {
        auto ln = line();
        auto cl = column();
        auto name = current_token;

        eat(ID);

        eat(LCUR);

        std::vector<std::shared_ptr<Node>> variants;
        while (current_token.tp != RCUR) {
            ignore_nl();
            auto variant = VariableNode::create(current_token);
            eat(ID);
            variant->line_number = line();
            variant->column_number = column();

            variants.push_back(std::move(variant));

            ignore_nl();
            if (current_token.tp != RCUR) {
                eat(COMMA);
            }
        }
        eat(RCUR);

        auto result = EnumNode::create(name, std::move(variants));
        result->line_number = ln;
        result->column_number = cl;
        return result;
    }

    std::shared_ptr<Node> Parser::classstatement() {
        auto ln = line();
        auto cl = column();
        auto name = current_token;

        eat(ID);

        auto inherits = NoOpNode::create();

        if (current_token.tp == COLON) {
            eat(COLON);
            inherits = get_type();
        }

        eat(LCUR);
        auto body = class_body();
        eat(RCUR);

        auto result = ClassNode::create(name, inherits, body);
        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

    std::shared_ptr<Node> Parser::newInitializer() {
        auto ln = line();
        auto cl = column();
        auto clss = get_type();

        std::vector<std::shared_ptr<Node>> argList;
        if (current_token.tp == LPAR) {
            eat(LPAR);

            argList = call_args();

            eat(RPAR);
        }

        auto result = ClassInitializerNode::create(clss, argList);

        result->line_number = ln;
        result->column_number = cl;
        return result;
    }

    std::shared_ptr<Node> Parser::constructor() {
        auto ln = line();
        auto cl = column();
        eat(LPAR);
        auto params = parameters();
        eat(RPAR);

        eat(LCUR);
        auto bl = func_body();
        eat(RCUR);

        auto result = ConstructorDeclNode::create(params, bl);

        result->line_number = ln;
        result->column_number = cl;
        return result;
    }

    std::shared_ptr<Node> Parser::loopstatement() {
        auto ln = line();
        auto cl = column();
        std::shared_ptr<Node> body;

        if (current_token.tp == LCUR) {
            eat(LCUR);
            body = block();
            eat(RCUR);
        } else {
            body = statement();
        }

        auto result = LoopNode::create(body);

        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

    std::shared_ptr<Node> Parser::whilestatement() {
        auto ln = line();
        auto cl = column();

        auto comp = ternary_op();

        std::shared_ptr<Node> body;
        if (current_token.tp == LCUR) {
            eat(LCUR);
            body = block();
            eat(RCUR);
        } else {
            body = statement();
        }

        auto result = WhileNode::create(comp, body);

        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

    std::shared_ptr<Node> Parser::forstatement() {
        auto ln = line();
        auto cl = column();

        eat(LPAR);
        auto initializer = statement();

        auto compar = ternary_op();
        statement_terminator();
        auto inc = statement();

        eat(RPAR);

        std::shared_ptr<Node> body;

        if (current_token.tp == LCUR) {
            eat(LCUR);
            body = block();
            eat(RCUR);
        } else {
            body = statement();
        }

        auto result = ForNode::create(initializer, compar, inc, body);

        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

    std::shared_ptr<Node> Parser::foreachstatement() {
        auto ln = line();
        auto cl = column();

        bool has_paren = current_token.tp == LPAR;
        ignore_nl();
        if (has_paren){
            eat(LPAR);
            ignore_nl();
        }

        auto var = current_token;
        if (has_paren)
            ignore_nl();
        eat(ID);
        ignore_nl();

        Lexing::Token reverse_token(NOTHING, "");
        if (current_token.tp == REV) {
            eat(REV);
            reverse_token = {REV, "~"};
            ignore_nl();
        }

        eat(COLON);
        ignore_nl();

        auto lst_expression = ternary_op();

        if (has_paren) {
            eat(RPAR);
            ignore_nl();
        }

        std::shared_ptr<Node> body;

        body = statement(false);

        auto result = ForEachNode::create(std::move(var), std::move(lst_expression), body, std::move(reverse_token));

        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

    std::shared_ptr<Node> Parser::forangestatement() {
        auto ln = line();
        auto cl = column();

        bool has_paren = current_token.tp == LPAR;
        ignore_nl();
        if (has_paren){
            eat(LPAR);
            ignore_nl();
        }

        Token var(NOTHING, "");
        if (current_token.tp == ID) {
            var = current_token;
            eat(ID);
        }
        ignore_nl();

        Lexing::Token reverse_token(NOTHING, "");
        if (current_token.tp == REV) {
            eat(REV);
            reverse_token = {REV, "~"};
            ignore_nl();
        }

        eat(COLON);
        ignore_nl();

        auto first_expression = ternary_op();
        std::shared_ptr<Node> second_expression;

        if (current_token.tp == COMMA) {
            ignore_nl();
            eat(COMMA);
            ignore_nl();
            second_expression = ternary_op();
        }

        if (has_paren) {
            ignore_nl();
            eat(RPAR);
            ignore_nl();
        }

        std::shared_ptr<Node> body;

        body = statement(false);

        auto result = FoRangeNode::create(var, first_expression, second_expression, body, reverse_token);

        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

    std::shared_ptr<Node> Parser::ifstatement() {
        auto ln = line();
        auto cl = column();

        auto exp = ternary_op();

        if (current_token.tp == LCUR) {
            eat(LCUR);

            auto trueBlock = block();
            eat(RCUR);

            if (current_token.tp == ELSE) {
                eat(ELSE);

                std::shared_ptr<Node> elseBlock;

                if (current_token.tp == LCUR) {
                    eat(LCUR);
                    elseBlock = block();
                    eat(RCUR);
                } else {
                    eat(IF);
                    elseBlock = ifstatement();
                }

                auto result = IfNode::create(exp, trueBlock, elseBlock);

                result->line_number = ln;
                result->column_number = cl;

                return result;
            }

            auto result = IfNode::create(exp, trueBlock, nullptr);

            result->line_number = ln;
            result->column_number = cl;

            return result;
        } else {
            auto result = IfNode::create(exp, statement(), nullptr);

            result->line_number = ln;
            result->column_number = cl;

            return result;
        }
    }

    std::shared_ptr<Node> Parser::function() {
        auto ln = line();
        auto cl = column();

        auto name = current_token;
        eat(ID);

        eat(LPAR);
        auto params = parameters();
        eat(RPAR);

        auto retType = NoOpNode::create();
        if (current_token.tp == COLON) {
            eat(COLON);
            retType = get_full_type();

//            while (current_token.tp == LBRA) {
//                eat(LBRA);
////                retType.value += "[]";
//                eat(RBRA);
//            }
        }

        eat(LCUR);
        auto bl = func_body();
        eat(RCUR);

        auto result = FuncDeclNode::create(name, params, retType, bl);

        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

    std::vector<std::shared_ptr<Node>> Parser::parameters() {
        std::vector<std::shared_ptr<Node>> params;

        if (current_token.tp != RPAR) {
            params.push_back(declaration(true));

            while (current_token.tp != RPAR) {
                eat(COMMA);
                params.push_back(declaration(true));
            }
        }

        return params;
    }

    std::vector<std::shared_ptr<Node>> Parser::call_args() {
        std::vector<std::shared_ptr<Node>> argList;

        while (current_token.tp != RPAR) {
            argList.push_back(ternary_op());

            ignore_nl();
            if (current_token.tp != RPAR) {
                eat(COMMA);
            }
            ignore_nl();
        }

        return argList;
    }

    std::shared_ptr<Node> Parser::declaration(bool force_type) {
        auto ln = line();
        auto cl = column();

        auto varName = current_token;
        eat(ID);

        std::shared_ptr<Node> assignment{nullptr};

//        bool using_direct_init = false;

        std::shared_ptr<Node> tp;
        int list_dimensions = 0;

        if (force_type || current_token.tp == COLON) {
            eat(COLON);
            tp = get_full_type();

            while (tp->kind() == NodeType::Index) {
                tp = Node::as<IndexNode>(tp)->val;
                list_dimensions++;
            }
        } else {
            tp = VariableNode::create(Token(ID, "any"));
        }

//        if (tp.is lists?)

//        if (current_token.tp == LBRA) {
//            eat(LBRA);
//            eat(RBRA);
//
//            int dimensions = 1;
//            while (current_token.tp == LBRA) {
//                eat(LBRA);
//                dimensions++;
//                eat(RBRA);
//            }
//
//            if (current_token.tp == ASS) {
//                eat(ASS);
//                assignment = ternary_op();
//            }
//
//            auto result = ListDeclarationNode::create(tp, varName, dimensions, assignment);
//
//            result->line_number = ln;
//            result->column_number = cl;
//
//            return result;
//        } else if (current_token.tp == LPAR) {
//            using_direct_init = true;
//            eat(LPAR);
//            auto pars = call_args();
//            eat(RPAR);
//            assignment = ClassInitializerNode::create(tp, pars);
//        }

        if (current_token.tp == ASS) {
//            if (using_direct_init) {
//                std::string err_msg = INVALID_DIRECT_EXCP;
//                throw Exceptions::OdoException(err_msg, lexer.getCurrentLine(), lexer.getCurrentCol());
//            }
            eat(ASS);
            assignment = ternary_op();
        }

        std::shared_ptr<Node> result;
        if (list_dimensions > 0) {
            result = ListDeclarationNode::create(tp, varName, list_dimensions, assignment);
        } else {
            result = VarDeclarationNode::create(tp, varName, assignment);
        }

        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

//    std::shared_ptr<Node> Parser::declaration(std::shared_ptr<Node> tp) {
//        auto ln = line();
//        auto cl = column();
//
//        auto varName = current_token;
//        eat(ID);
//
//        std::shared_ptr<Node> assignment{nullptr};
//
//        bool using_direct_init = false;
//
//        if (current_token.tp == LBRA) {
//            eat(LBRA);
//            eat(RBRA);
//
//            int dimensions = 1;
//            while (current_token.tp == LBRA) {
//                eat(LBRA);
//                dimensions++;
//                eat(RBRA);
//            }
//
//            if (current_token.tp == ASS) {
//                eat(ASS);
//                assignment = ternary_op();
//            }
//
//            auto result = ListDeclarationNode::create(tp, varName, dimensions, assignment);
//
//            result->line_number = ln;
//            result->column_number = cl;
//
//            return result;
//        } else if (current_token.tp == LPAR) {
//            using_direct_init = true;
//            eat(LPAR);
//            auto pars = call_args();
//            eat(RPAR);
//            assignment = ClassInitializerNode::create(tp, pars);
//        }
//
//        if (current_token.tp == ASS) {
//            if (using_direct_init) {
//                std::string err_msg = INVALID_DIRECT_EXCP;
//                throw Exceptions::OdoException(err_msg, lexer.getCurrentLine(), lexer.getCurrentCol());
//            }
//            eat(ASS);
//            assignment = ternary_op();
//        }
//
//        auto result = VarDeclarationNode::create(tp, varName, assignment);
//
//        result->line_number = ln;
//        result->column_number = cl;
//
//        return result;
//    }

    std::shared_ptr<Node> Parser::ternary_op() {
        ignore_nl();
        auto ln = line();
        auto cl = column();

        auto comp = or_comparison();

        while (current_token.tp == QUEST) {
            eat(QUEST);
            auto trueSection = or_comparison();

            eat(COLON);
            auto falseSection = or_comparison();

            auto result = TernaryOpNode::create(comp, trueSection, falseSection);
            result->line_number = ln;
            result->column_number = cl;

            ln = line();
            cl = column();

            comp = result;
        }

        return comp;
    }

    std::shared_ptr<Node> Parser::or_comparison() {
        auto ln = line();
        auto cl = column();
        auto node = and_comparison();

        while (current_token.tp == OR) {
            auto or_tk = current_token;
            eat(OR);

            auto result = BinOpNode::create(or_tk, node, and_comparison());

            result->line_number = ln;
            result->column_number = cl;

            ln = line();
            cl = column();
            node = result;
        }

        return node;
    }

    std::shared_ptr<Node> Parser::and_comparison() {
        auto ln = line();
        auto cl = column();

        auto node = comparison();

        while (current_token.tp == AND) {
            auto and_tk = current_token;
            eat(AND);

            auto result = BinOpNode::create(and_tk, node, comparison());
            result->line_number = ln;
            result->column_number = cl;

            ln = line();
            cl = column();
            node = result;
        }
        return node;
    }

    std::shared_ptr<Node> Parser::comparison() {
        auto ln = line();
        auto cl = column();

        auto exp = expression();
        auto compType = {EQU, NEQ, GT, LT, LET, GET};

        while (contains_type(compType, current_token.tp)) {
            auto c_token = current_token;
            eat(current_token.tp);
            auto result = BinOpNode::create(c_token, exp, expression());

            result->line_number = ln;
            result->column_number = cl;

            ln = line();
            cl = column();
            exp = result;
        }

        return exp;
    }

    std::shared_ptr<Node> Parser::expression() {
        auto ln = line();
        auto cl = column();
        auto node = term();

        auto exprType = {PLUS, MINUS};

        while (contains_type(exprType, current_token.tp)) {
            auto c_token = current_token;

            eat(current_token.tp);

            auto result = BinOpNode::create(c_token, node, term());

            result->line_number = ln;
            result->column_number = cl;

            ln = line();
            cl = column();

            node = result;
        }

        return node;
    }

    std::shared_ptr<Node> Parser::term() {
        auto ln = line();
        auto cl = column();

        auto node = prod();

        auto termType = {MUL, DIV, MOD};

        while (contains_type(termType, current_token.tp)) {
            auto c_token = current_token;

            eat(current_token.tp);

            auto result = BinOpNode::create(c_token, node, prod());

            result->line_number = ln;
            result->column_number = cl;

            ln = line();
            cl = column();

            node = result;
        }

        return node;
    }

    std::shared_ptr<Node> Parser::prod() {
        auto ln = line();
        auto cl = column();

        auto node = postfix();

        while (current_token.tp == POW) {
            auto c_token = current_token;

            eat(POW);
            auto result = BinOpNode::create(c_token, node, postfix());

            result->line_number = ln;
            result->column_number = cl;

            ln = line();
            cl = column();

            node = result;
        }

        return node;
    }

    std::shared_ptr<Node> Parser::postfix() {
        auto ln = line();
        auto cl = column();

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
            std::shared_ptr<Node> result;
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
                    auto operation = BinOpNode::create(binOP, node, IntNode::create(one));

                    operation->line_number = ln;
                    operation->column_number = cl;

                    ln = line();
                    cl = column();

                    result = AssignmentNode::create(node, operation);

                    result->line_number = ln;
                    result->column_number = cl;

                    ln = line();
                    cl = column();

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

                    auto operation = BinOpNode::create(binOP, node, expression());

                    operation->line_number = ln;
                    operation->column_number = cl;

                    ln = line();
                    cl = column();

                    result = AssignmentNode::create(node, operation);

                    result->line_number = ln;
                    result->column_number = cl;

                    ln = line();
                    cl = column();

                    node = result;
                    break;
                }
                case DOT:
                {
                    eat(DOT);
                    result = MemberVarNode::create(node, current_token);

                    eat(ID);

                    result->line_number = ln;
                    result->column_number = cl;

                    ln = line();
                    cl = column();
                    node = result;
                    break;
                }
                case DCOLON:
                {
                    eat(DCOLON);
                    result = StaticVarNode::create(node, current_token);

                    eat(ID);

                    result->line_number = ln;
                    result->column_number = cl;

                    ln = line();
                    cl = column();

                    node = result;
                    break;
                }
                case LPAR:
                {
                    eat(LPAR);

                    auto argList = call_args();

                    eat(RPAR);
                    Token name(NOTHING, "");
                    if (node && node->kind() == NodeType::Variable){
                        name = Node::as<VariableNode>(node)->token;
                    }

                    result = FuncCallNode::create(node, name, argList);

                    result->line_number = ln;
                    result->column_number = cl;

                    ln = line();
                    cl = column();

                    node = result;
                    break;
                }
                case LBRA:
                    eat(LBRA);

                    result = IndexNode::create(node, ternary_op());

                    result->line_number = ln;
                    result->column_number = cl;

                    ln = line();
                    cl = column();

                    node = result;
                    eat(RBRA);
                    break;
                case ASS:
                    eat(ASS);

                    result = AssignmentNode::create(node, ternary_op());

                    result->line_number = ln;
                    result->column_number = cl;

                    ln = line();
                    cl = column();

                    node = result;
                    break;
                default:
                    break;
            }
        }

//        if (current_token.tp == ID) {
//            node = declaration(node);
//        }

        return node;
    }

    std::shared_ptr<Node> Parser::funcexpression() {
        auto ln = line();
        auto cl = column();

        eat(LPAR);
        auto params = parameters();

        eat(RPAR);

        std::shared_ptr<Node> body;
        if (current_token.tp == LCUR) {
            eat(LCUR);
            body = func_body();
            eat(RCUR);
        } else {
            eat(ARROW);
            auto val = ternary_op();
            auto ex = ReturnNode::create(val);
            ex->line_number = ln;
            ex->column_number = cl;

            std::vector<std::shared_ptr<Node>> bod_list{ex};

            std::shared_ptr<Node> fbod = FuncBodyNode::create(bod_list);
            fbod->line_number = ln;
            fbod->column_number = cl;

            body = fbod;
        }

        auto result = FuncExpressionNode::create(params, NoOpNode::create(), body);

        result->line_number = ln;
        result->column_number = cl;

        return result;
    }

    std::shared_ptr<Node> Parser::factor() {
        ignore_nl();
        switch (current_token.tp) {
            case REAL:
            {
                auto double_literal = DoubleNode::create(current_token);
                eat(REAL);
                return double_literal;
            }
            case INT:
            {
                auto int_literal = IntNode::create(current_token);
                eat(INT);
                return int_literal;
            }
            case STR:
            {
                auto str_literal = StrNode::create(current_token);
                eat(STR);
                return str_literal;
            }
            case BOOL:
            {
                auto bool_literal = BoolNode::create(current_token);
                eat(BOOL);
                return bool_literal;
            }
            case NOT:
            {
                eat(NOT);
                auto c = ternary_op();
                auto result = std::make_shared<BinOpNode>(
                    Token(EQU, "=="),
                    std::make_shared<BoolNode>(Token(BOOL, FALSE_TK)),
                    c
                );

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
                auto op = current_token;
                eat(current_token.tp);
                auto result = UnaryOpNode::create(op, postfix());
                return result;
            }
            case NEW:
                eat(NEW);
                return newInitializer();
            case ID:
            {
                auto idToken = current_token;
                eat(ID);

                return VariableNode::create(idToken);
            }
            case LBRA:
            {
                eat(LBRA);

                std::vector<std::shared_ptr<Node>> contents;
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
                auto list_expr = ListExpressionNode::create(contents);
                return list_expr;
            }
            case NULLT:
            {
                auto null_ret = NullNode::create();
                eat(NULLT);
                return null_ret;
            }
            case FUNC:
                eat(FUNC);
                return funcexpression();
            default:
                break;
        }

        return NoOpNode::create();
    }

    Parser::Parser(): lexer(Lexer()) {}

    std::vector<std::shared_ptr<Node>> Parser::program_content() {
        return statement_list();
    }

    std::shared_ptr<Node> Parser::add_dbg_info(std::shared_ptr<Node> ins) {
        ins->line_number = lexer.getCurrentLine();
        ins->column_number = lexer.getCurrentCol();
        return ins;
    }
}
