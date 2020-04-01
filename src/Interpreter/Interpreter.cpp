//
// Created by Luis Gonzalez on 3/20/20.
//

#include "Interpreter/Interpreter.h"

#include <cmath>
#include <iostream>

Interpreter::Interpreter(Parser p): parser(std::move(p)) {
    any_type = {.name="any", .isType=true, .kind=PrimitiveType};

    std::map<std::string, Symbol> buildInTypes = {
        {"any", any_type},
        {"int", {.name = "int", .tp = &any_type, .isType=true, .kind=PrimitiveType}},
        {"double", {.name = "double", .tp = &any_type, .isType=true, .kind=PrimitiveType}},
        {"string", {.name = "string", .tp = &any_type, .isType=true, .kind=PrimitiveType}},
        {"bool", {.name = "bool", .tp = &any_type, .isType=true, .kind=PrimitiveType}},
        {"pointer", {.name = "pointer", .tp = &any_type, .isType=true, .kind=PrimitiveType}},
        {"NullType", {.name = "NullType", .tp = &any_type, .isType=true, .kind=PrimitiveType}}
    };

    globalTable = SymbolTable("global", buildInTypes);
    currentScope = &globalTable;

    replScope = SymbolTable("repl", {}, &globalTable);

    valueTable = ValueTable();

    null = valueTable.addNewValue(
        buildInTypes["NullType"],
        "null"
    );

    auto nullSym = Symbol{
        .name = "null",
        .tp = &buildInTypes["NullType"],
        .value = null
    };

    globalTable.addSymbol(nullSym);

    returning = nullptr;

    add_native_function("print", [&](auto values) {
        for (auto v : values) {
            std::cout << value_to_string(*v);
        }
        return null;
    });

    add_native_function("println", [&](auto values) {
        for (auto v : values) {
            std::cout << value_to_string(*v);
        }
        std::cout << "\n";
        return null;
    });

    add_native_function("factorial", [&](auto v){
        if (!v.empty() && v[0]->type.name == "int") {
            int arg1 = v[0]->as_int();
            int result = 1;
            for(int i = 1; i <= arg1; i++) {
                result = result * i;
            }
            return create_literal(std::to_string(result), "int");
        }
        std::cout << "factorial function should be called with 1 int argument.";
        throw 1;
        return null;
    });

    // Something about setting up the character that clears the screen?
}

Value* Interpreter::visit(AST node) {
    switch (node.tp) {
        // Normal Types
        case Double:
            return visit_Double(node.token);
        case Int:
            return visit_Int(node.token);
        case Bool:
            return visit_Bool(node.token);
        case Str:
            return visit_Str(node.token);

        // Operations
        case BinOp:
            return visit_BinOp(node.token, node.nodes["left"], node.nodes["right"]);
            break;
        case UnaryOp:
            return visit_UnaryOp(node.token, node.nodes["right"]);
            break;
        case NoOp:
            return null;

        case Index:
//            return visit_Index(nodes.nodes["val"], node.nodes["right"]);
            break;

        // Control Flow
        case TernaryOp:
            return visit_TernaryOp(node.nodes["cond"], node.nodes["trueb"], node.nodes["falseb"]);
            break;
        case If:
            return visit_If(node.nodes["expr"], node.nodes["trueb"], node.nodes["falseb"]);
            break;
        case For:
            return visit_For(node.nodes["ini"], node.nodes["cond"], node.nodes["incr"], node.nodes["body"]);
            break;
        case While:
            return visit_While(node.nodes["cond"], node.nodes["body"]);
            break;
        case Loop:
            return visit_Loop(node.nodes["body"]);
            break;
        case Break:
            breaking = true;
            return null;
        case Continue:
            continuing = true;
            return null;
        case Block:
            return visit_Block(node.lst_AST);

        // Variable Handling
        case Declaration:
            return visit_VarDeclaration(node.type, node.token, node.nodes["initial"]);
        case ListDeclaration:
//            return visit_ListDeclaration(node.type, node.token, node.nodes["initial"]);
            break;
        case Variable:
            return visit_Variable(node.token);
            break;
        case Assignment:
            return visit_Assignment(node.nodes["token"], node.nodes["right"]);
            break;

        case ListExpression:
//            return visit_ListExpression(node.lst_AST);
            break;

        // Functions
        case FuncExpression:
//            return visit_FuncExpression(node.lst_AST, node.type, node.nodes["body"]);
            break;
        case FuncDecl:
//            return visit_FuncExpression(node.token, node.lst_AST, node.type, node.nodes["body"]);
            break;
        case FuncCall:
            return visit_FuncCall(node.nodes["fun"], node.token, node.lst_AST);
            break;
        case FuncBody:
//            return visit_FuncBody(node.lst_AST);
            break;
        case Return:
//            return visit_Return(node.lst_AST, node.type, node.nodes["body"]);
            break;

        // Classes
        case Class:
//            return visit_Class(node.token, node.type, node.nodes["body"]);
            break;
        case ClassBody:
//            return visit_ClassBody(node.lst_AST);
            break;
        case ConstructorDecl:
//            return visit_ConstructorDecl(node.nodes["params"], node.lst_AST);
            break;
        case ConstructorCall:
//            return visit_ConstructorCall(node.token);
            break;
        case InstanceBody:
//            return visit_InstanceBody(node.lst_AST);
            break;
        case ClassInitializer:
//            return visit_ClassInitializer(node.token, node.lst_AST);
            break;
        case StaticStatement:
//            return visit_StaticStatement(node.nodes["statement"]);
            break;
        case MemberVar:
//            return visit_MemberVar(node.nodes["inst"], node.token);
            break;
        case StaticVar:
//            return visit_StaticVar(node.nodes["inst"], node.token);
            break;

        case Debug:
//            cout << "Debugging\n";
            return null;

        case Null:
            return null;
    }
    return null;
}

std::string Interpreter::value_to_string(Value v) {
    std::string result;
    if (v.type.kind == PrimitiveType){
        if (v.type == *globalTable.findSymbol("double")) {
            auto as_double = v.as_double();
            result = std::to_string(as_double);
        } else if (v.type == *globalTable.findSymbol("int")){
            auto as_int = v.as_int();
            result = std::to_string(as_int);
        } else if (v.type == *globalTable.findSymbol("string")){
            result = v.as_string();
        } else if (v.type == *globalTable.findSymbol("bool")){
            auto as_bool = v.as_bool();
            result = as_bool ? "true" : "false";
        } else if (v.type == null->type){
            result = "null";
        } else {
            // Error!
            throw 1;
        }
    } else {
        result = "<value> at:" + std::to_string(v.address);
    }

    return result;
}

int Interpreter::add_native_function(const std::string& name, NativeFunction callback) {
    auto result = native_functions.find(name);
    if (result != native_functions.end())
        return -1;

    native_functions[name] = std::move(callback);
    return 0;
}

Value* Interpreter::create_literal(std::string val, const std::string& kind) {
    std::any newValue;
    if (kind == "double") {
        newValue = strtod(val.c_str(), nullptr);
    } else if (kind == "int") {
        int a = strtol(val.c_str(), nullptr, 10);
        newValue = a;
    } else if (kind == "string") {
        newValue = val;
    } else if (kind == "bool") {
        if (val != "true" && val != "false"){
            // Error!
            throw 1;
        }
        newValue = val == "true";
    } else {
        return null;
    }

    // Handle errors in conversions are incorrect.

    auto the_value = valueTable.addNewValue(
            *globalTable.findSymbol(kind),
            newValue
    );

    return the_value;
}

Value* Interpreter::visit_Double(Token t) {
    return create_literal(t.value, "double");
}

Value* Interpreter::visit_Int(Token t) {
    return create_literal(t.value, "int");
}

Value* Interpreter::visit_Bool(Token t) {
    return create_literal(t.value, "bool");
}

Value* Interpreter::visit_Str(Token t) {
    return create_literal(t.value, "string");
}

Value* Interpreter::visit_Block(std::vector<AST> statements) {
    auto blockScope = SymbolTable("block_scope", {}, currentScope);
    currentScope = &blockScope;

    auto result = null;
    for (auto st : statements) {
        result = visit(st);
        if (breaking || continuing || returning) {
            break;
        }
    }

    currentScope = blockScope.getParent();
     valueTable.cleanUp(blockScope);

    return result;
}

Value* Interpreter::visit_TernaryOp(AST cond, AST trueb, AST falseb) {
    auto val_cond = visit(cond);
    bool real_condition = std::any_cast<bool>(val_cond->val);

    // TODO: Handle Errors
    // TODO: How to check if value is actually bool?
    if (real_condition) {
        return visit(trueb);
    } else {
        return visit(falseb);
    }
    return nullptr;
}

Value* Interpreter::visit_If(AST cond, AST trueb, AST falseb) {
    auto val_cond = visit(cond);
    bool real_condition = std::any_cast<bool>(val_cond->val);

    // TODO: Handle Errors
    // TODO: How to check if value is actually bool?
    if (real_condition) {
        return visit(trueb);
    } else {
        return visit(falseb);
    }
    return nullptr;
}

Value* Interpreter::visit_For(AST ini, AST cond, AST incr, AST body) {
    auto forScope = SymbolTable("for:loop", {}, currentScope);
    currentScope = &forScope;

    visit(std::move(ini));
    // TODO: Handle error
    // Error if cond is not bool
    while(visit(cond)->as_bool()){
        if (continuing) {
            continuing = false;
            continue;
        } else {
            visit(body);
        }

        if (breaking) {
            breaking = false;
            break;
        }

        if (returning) {
            break;
        }

        visit(incr);
    }

    currentScope = forScope.getParent();
    valueTable.cleanUp(forScope);

    return null;
}

Value *Interpreter::visit_While(const AST& cond, AST body) {
    auto whileScope = SymbolTable("while:loop", {}, currentScope);
    currentScope = &whileScope;

    // TODO: Handle error
    // Error if cond is not bool
    while(visit(cond)->as_bool()){
        visit(body);
        if (breaking) {
            breaking = false;
            break;
        }
        if (continuing) {
            continuing = false;
            continue;
        }
        if (returning) {
            break;
        }
    }

    currentScope = whileScope.getParent();
    valueTable.cleanUp(whileScope);

    return null;
}

Value *Interpreter::visit_Loop(AST body) {
    // TODO: Handle error
    // Error if cond is not bool
    while(true){
        visit(body);
        if (breaking) {
            breaking = false;
            break;
        }
        if (continuing) {
            continuing = false;
            continue;
        }
        if (returning) {
            break;
        }
    }

    return null;
}

Value* Interpreter::visit_VarDeclaration(const Token& var_type, Token name, AST initial) {
    if (currentScope->symbolExists(name.value)) {
        // TODO: Handle Errors
        // NameError! Duplicate Variable name.value
    } else {
        auto type_ = currentScope->findSymbol(var_type.value);

        if (type_ == nullptr) {
            // TODO: Handle Errors
            // NameError! Unknown identifier var_type.value
        }

        Symbol newVar;
        Value* valueReturn;

        if (initial.tp != NoOp) {
            auto newValue = visit(initial);

            if (newValue->type.kind == PrimitiveType) {
                newValue = valueTable.copyValue(*newValue);
            }

            newVar = {
                type_,
                name.value,
                newValue
            };

            valueReturn = newValue;
        } else {
            newVar = {
                type_,
                name.value
            };

            valueReturn = null;
        }

        auto in_sym_table = currentScope->addSymbol(newVar);
        if (valueReturn != null) {
            valueReturn->addReference(*in_sym_table);
        }
        return valueReturn;
    }
    return null;
}

// TODO: add getMemberVar function.
Value* Interpreter::visit_Assignment(AST expr, AST val) {
    auto varSym = getMemberVarSymbol(expr);
    auto newValue = visit(val);

    if (varSym) {
        if (varSym->value) {
            auto theValue = varSym->value;

            theValue->removeReference(*varSym);
            if (newValue->type.kind == PrimitiveType) {
                newValue = valueTable.addNewValue(newValue->type, newValue->val);
            }

            varSym->value = newValue;
            newValue->addReference(*varSym);
        } else {
            if (newValue->type.kind == PrimitiveType) {
                newValue = valueTable.addNewValue(newValue->type, newValue->val);
            }

            varSym->value = newValue;
            newValue->addReference(*varSym);
        }
    } else {
        // TODO: Handle Error
        // Error! Assignment to unknown variable.
    }
    return null;
}

Value* Interpreter::visit_Variable(Token token) {
    auto found = currentScope->findSymbol(token.value);

    if (found != nullptr) {
        if (found->value) {
            auto val = found->value;

            return (val == nullptr) ? null : val;
        } else {
            return null;
        }
    } else {
        // TODO: Handle Errors
        // Error! Variable Named token.value was not found.
        throw 1;
    }
}

Value* Interpreter::visit_BinOp(Token token, AST &left, AST &right) {
    auto leftVisited = visit(left);
    auto rightVisited = visit(right);

    auto leftVal = leftVisited->val;
    auto rightVal = rightVisited->val;

    switch (token.tp) {
        case PLUS:
            // TODO: left or right as Lists
            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() + rightVisited->as_int();
                    return create_literal(std::to_string(result), "int");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() + rightVisited->as_double();
                    return create_literal(std::to_string(result), "double");
                }
            } else if (leftVisited->type.name == "string") {
                return create_literal(leftVisited->as_string()+value_to_string(*rightVisited), "string");
            } else if (rightVisited->type.name == "string") {
                return create_literal(value_to_string(*leftVisited) + rightVisited->as_string(), "string");
            } else {
                // TODO: Handle Error
                // ValueError! Numeric adition must be with operands of the same type
                throw 1;
            }
            break;
        case MINUS:
            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() - rightVisited->as_int();
                    return create_literal(std::to_string(result), "int");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() - rightVisited->as_double();
                    return create_literal(std::to_string(result), "double");
                }
            } else {
                // TODO: Handle Error
                // ValueError! Numeric substraction must be with operands of the same type
                throw 1;
            }
            break;
        case MUL:
            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() * rightVisited->as_int();
                    return create_literal(std::to_string(result), "int");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() * rightVisited->as_double();
                    return create_literal(std::to_string(result), "double");
                }
            // TODO: Handle cases in which left or right is a list
            } else {
                // TODO: Handle Error
                // ValueError! Numeric substraction must be with operands of the same type
                throw 1;
            }
            break;
        case DIV:
            if (leftVisited->type == rightVisited->type && leftVisited->type.name == "double") {
                auto result = leftVisited->as_double() / rightVisited->as_double();
                return create_literal(std::to_string(result), "double");
            }else {
                // TODO: Handle Error
                // ValueError! Division operation must be with operands of type double
                throw 1;
            }
            break;
        case POW:
            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = powl(leftVisited->as_int(), rightVisited->as_int());
                    return create_literal(std::to_string(result), "int");
                } else if (leftVisited->type.name == "double") {
                    auto result = powf(leftVisited->as_double(), rightVisited->as_double());
                    return create_literal(std::to_string(result), "double");
                }
            } else {
                // TODO: Handle Error
                // ValueError! Power operation must be with operands of the same type
                throw 1;
            }
            break;
        case EQU:
            if (leftVisited == rightVisited)
                return create_literal("true", "bool");

            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() == rightVisited->as_int();
                    return create_literal(result ? "true" : "false", "bool");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() == rightVisited->as_double();
                    return create_literal(result ? "true" : "false", "bool");
                } else if (leftVisited->type.name == "bool") {
                    auto result = leftVisited->as_bool() == rightVisited->as_bool();
                    return create_literal(result ? "true" : "false", "bool");
                } else if (leftVisited->type.name == "string") {
                    auto result = leftVisited->as_string() == rightVisited->as_string();
                    return create_literal(result ? "true" : "false", "bool");
                }
            } else {
                // TODO: Handle Error
                // ValueError! Comparison operation must be with operands of the same type
                throw 1;
            }
            break;
        case NEQ:
            if (leftVisited == rightVisited)
                return create_literal("false", "bool");

            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() != rightVisited->as_int();
                    return create_literal(result ? "true" : "false", "int");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() != rightVisited->as_double();
                    return create_literal(result ? "true" : "false", "double");
                } else if (leftVisited->type.name == "bool") {
                    auto result = leftVisited->as_bool() != rightVisited->as_bool();
                    return create_literal(result ? "true" : "false", "bool");
                } else if (leftVisited->type.name == "string") {
                    auto result = leftVisited->as_string() != rightVisited->as_string();
                    return create_literal(result ? "true" : "false", "bool");
                }
            } else {
                // TODO: Handle Error
                // ValueError! Comparison operation must be with operands of the same type
                throw 1;
            }
            break;
        case LT:
            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() < rightVisited->as_int();
                    return create_literal(result ? "true" : "false", "bool");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() < rightVisited->as_double();
                    return create_literal(result ? "true" : "false", "bool");
                }
            } else {
                // TODO: Handle Error
                // ValueError! Comparison operation must be with operands of the same type
                throw 1;
            }
            break;
        case GT:
            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() > rightVisited->as_int();
                    return create_literal(result ? "true" : "false", "bool");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() > rightVisited->as_double();
                    return create_literal(result ? "true" : "false", "bool");
                }
            } else {
                // TODO: Handle Error
                // ValueError! Comparison operation must be with operands of the same type
                throw 1;
            }
            break;
        case LET:
            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() <= rightVisited->as_int();
                    return create_literal(result ? "true" : "false", "bool");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() <= rightVisited->as_double();
                    return create_literal(result ? "true" : "false", "bool");
                }
            } else {
                // TODO: Handle Error
                // ValueError! Comparison operation must be with operands of the same type
                throw 1;
            }
            break;
        case GET:
            if (leftVisited->type == rightVisited->type) {
                if (leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() >= rightVisited->as_int();
                    return create_literal(result ? "true" : "false", "bool");
                } else if (leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() >= rightVisited->as_double();
                    return create_literal(result ? "true" : "false", "bool");
                }
            } else {
                // TODO: Handle Error
                // ValueError! Comparison operation must be with operands of the same type
                throw 1;
            }
            break;
        case AND:
            if (leftVisited->type == rightVisited->type && leftVisited->type.name == "bool") {
                auto result = leftVisited->as_bool() && rightVisited->as_bool();
                return create_literal(result ? "true" : "false", "double");
            }else {
                // TODO: Handle Error
                // ValueError! Division operation must be with operands of type double
                throw 1;
            }
            break;
        case OR:
            if (leftVisited->type == rightVisited->type && leftVisited->type.name == "bool") {
                auto result = leftVisited->as_bool() || rightVisited->as_bool();
                return create_literal(result ? "true" : "false", "double");
            }else {
                // TODO: Handle Error
                // ValueError! Division operation must be with operands of type double
                throw 1;
            }
            break;
        default:
            break;
    }
    return nullptr;
}

Value* Interpreter::visit_UnaryOp(Token token, AST &left) {
    auto result = visit(left);

    switch (token.tp) {
        case PLUS:
            break;
        case MINUS:
            if (result->type.name == "int") {
                auto actual_value = result->as_int();

                result->val = actual_value * -1;
                return result;
            }

            if (result->type.name == "double") {
                auto actual_value = result->as_double();

                result->val = actual_value * -1;
                return result;
            }
            break;
        default:
            break;
    }
}

Value *Interpreter::visit_FuncCall(AST expr, Token fname, std::vector<AST> args) {
    if (callDepth >= MAX_CALL_DEPTH) {
        //TODO: Handle Error
        // Error! Max call depth exceeded
    }

    if (fname.tp != NOTHING){
        auto found_in_natives = native_functions.find(fname.value);

        if (found_in_natives != native_functions.end()) {
            std::vector<Value*> arguments_visited;
            for(auto arg : args){
                arguments_visited.push_back(visit(arg));
            }

            return found_in_natives->second(arguments_visited);
        }
    }

    throw 1;
}

Symbol *Interpreter::getMemberVarSymbol(const AST& mem) {
    Symbol* varSym = nullptr;

    switch (mem.tp) {
        case Variable:
            varSym = currentScope->findSymbol(mem.token.value);
            break;
        case MemberVar:
            // TODO
            break;
        case StaticVar:
            // TODO
            break;
        case Index:
            // TODO
            break;
        default:
            break;
    }

    return varSym;
}
