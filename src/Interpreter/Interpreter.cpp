//
// Created by Luis Gonzalez on 3/20/20.
//

#include "Interpreter/Interpreter.h"

#include <cmath>
#include <iostream>
#include <random>
namespace Odo::Interpreting {
    using namespace Parsing;

    double rand_int(int min, int max) {
        std::default_random_engine generator(clock());
        std::uniform_int_distribution<int> distribution(min, max-1);
        return distribution(generator);
    }

    double rand_double(double min, double max) {
        std::default_random_engine generator(clock());
        std::uniform_real_distribution<double> distribution(min, max);
        return distribution(generator);
    }

    Interpreter::Interpreter(Parser p): parser(std::move(p)) {
        auto any_symbol = Symbol{.name="any", .isType=true, .kind=PrimitiveType};

        std::map<std::string, Symbol> buildInTypes = {
            {"any", any_symbol}
        };

        any_type = &any_symbol;

        buildInTypes["int"] = Symbol{.name = "int", .tp = any_type, .isType=true, .kind=PrimitiveType};
        buildInTypes["double"] = Symbol{.name = "double", .tp = any_type, .isType=true, .kind=PrimitiveType};
        buildInTypes["string"] = Symbol{.name = "string", .tp = any_type, .isType=true, .kind=PrimitiveType};
        buildInTypes["bool"] = Symbol{.name = "bool", .tp = any_type, .isType=true, .kind=PrimitiveType};
        buildInTypes["pointer"] = Symbol{.name = "pointer", .tp = any_type, .isType=true, .kind=PrimitiveType};
        buildInTypes["NullType"] = Symbol{.name = "NullType", .tp = any_type, .isType=true, .kind=PrimitiveType};

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
            // Might make printing slower... I don't know of a better way of doing this.
            std::cout.flush();
            return null;
        });

        add_native_function("println", [&](auto values) {
            for (auto v : values) {
                std::cout << value_to_string(*v);
            }
            std::cout << std::endl;
            return null;
        });

        add_native_function("move_cursor", [&](auto vals) {
            std::cout << "\033[" << vals[0]->as_int() << ";" << vals[1]->as_int() << "H";

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

        add_native_function("length", [&](std::vector<Value*> v){
            // TODO: Error Handling
            if (!v.empty()) {
                auto arg = v[0];
                if (arg->type.name == "string") {
                    size_t len = arg->as_string().size();
                    return create_literal(std::to_string(len), "int");
                } else if (arg->kind == ListVal) {
                    size_t len = arg->as_list_value().size();
                    return create_literal(std::to_string(len), "int");
                }
            }
            throw 1;
            return null;
        });

        add_native_function("pow", [&](auto vals) {
            if (vals.size() >= 2) {
                bool result_as_int = true;
                double a = 0, b = 0;

                if (vals[0]->type.name == "double") {
                    a = vals[0]->as_double();
                    result_as_int = false;
                } else if (vals[0]->type.name == "int") {
                    a = vals[0]->as_int();
                }

                if (vals[1]->type.name == "double") {
                    b = vals[1]->as_double();
                    result_as_int = false;
                } else if (vals[1]->type.name == "int") {
                    b = vals[1]->as_int();
                }

                if (result_as_int) {
                    return create_literal(std::to_string((int)trunc(pow(a, b))), "int");
                } else {
                    return create_literal(std::to_string(pow(a, b)), "double");
                }
            }
            return null;
        });

        add_native_function("sqrt", [&](auto vals) {
            if (!vals.empty()) {
                double a = 0;

                if (vals[0]->type.name == "double") {
                    a = vals[0]->as_double();
                } else if (vals[0]->type.name == "int") {
                    a = vals[0]->as_int();
                }

                return create_literal(std::to_string(sqrt(a)), "double");
            }
            return null;
        });
        add_native_function("sin", [&](auto vals) {
            if (!vals.empty()) {
                double a = 0;

                if (vals[0]->type.name == "double") {
                    a = vals[0]->as_double();
                } else if (vals[0]->type.name == "int") {
                    a = vals[0]->as_int();
                }

                return create_literal(std::to_string(sin(a)), "double");
            }
            return null;
        });
        add_native_function("cos", [&](auto vals) {
            if (!vals.empty()) {
                double a = 0;

                if (vals[0]->type.name == "double") {
                    a = vals[0]->as_double();
                } else if (vals[0]->type.name == "int") {
                    a = vals[0]->as_int();
                }

                return create_literal(std::to_string(cos(a)), "double");
            }
            return null;
        });

        add_native_function("read", [&](std::vector<Value*> vals) {
            std::string result;
            for (auto v : vals) {
                std::cout << value_to_string(*v);
            }

            std::getline(std::cin, result);
            return create_literal(result, "string");
        });

        add_native_function("read_int", [&](std::vector<Value*> vals) {
            int result;
            for (auto v : vals) {
                std::cout << value_to_string(*v);
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(std::to_string(result), "int");
        });

        add_native_function("read_double", [&](std::vector<Value*> vals) {
            double result;
            for (auto v : vals) {
                std::cout << value_to_string(*v);
            }
            std::cin >> result;
            std::cin.ignore();
            return create_literal(std::to_string(result), "double");
        });

        add_native_function("rand", [&](auto vals) {
            double min = 0.0;
            double max = 1.0;
            if (vals.size() == 1) {
                if (vals[0]->type.name == "double") {
                    max = vals[0]->as_double();
                } else if (vals[0]->type.name == "int") {
                    max = vals[0]->as_int();
                }
            } else if (vals.size() >= 2) {
                if (vals[0]->type.name == "double") {
                    min = vals[0]->as_double();
                } else if (vals[0]->type.name == "int") {
                    min = vals[0]->as_int();
                }

                if (vals[1]->type.name == "double") {
                    max = vals[1]->as_double();
                } else if (vals[1]->type.name == "int") {
                    max = vals[1]->as_int();
                }
            }

            return create_literal(std::to_string(rand_double(min, max)), "double");
        });

        add_native_function("randInt", [&](auto vals) {
            int min = 0;
            int max = INT32_MAX;
            if (vals.size() == 1) {
                max = vals[0]->as_int();
            } else if (vals.size() >= 2) {
                min = vals[0]->as_int();
                max = vals[1]->as_int();
            }

            return create_literal(std::to_string(rand_int(min, max)), "int");
        });

        add_native_function("clear", [&](auto){std::cout << "\033[2J\033[1;1H"; return null;});

        add_native_function("wait", [&](auto){ std::cin.get(); return null; });

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
                return visit_Index(node.nodes["val"], node.nodes["expr"]);
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
                return visit_ListDeclaration(node.type, node.token, node.nodes["initial"]);
                break;
            case Variable:
                return visit_Variable(node.token);
                break;
            case Assignment:
                return visit_Assignment(node.nodes["token"], node.nodes["right"]);
                break;

            case ListExpression:
                return visit_ListExpression(node.lst_AST);
                break;

            // Functions
            case FuncExpression:
                return visit_FuncExpression(node.lst_AST, node.type, node.nodes["body"]);
                break;
            case FuncDecl:
                return visit_FuncDecl(node.token, node.lst_AST, node.type, node.nodes["body"]);
                break;
            case FuncCall:
                return visit_FuncCall(node.nodes["fun"], node.token, node.lst_AST);
                break;
            case FuncBody:
                return visit_FuncBody(node.lst_AST);
                break;
            case Return:
                return visit_Return(node.nodes["val"]);
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
            if (v.type.name == "double") {
                auto as_double = v.as_double();
                result = std::to_string(as_double);
            } else if (v.type.name == "int"){
                auto as_int = v.as_int();
                result = std::to_string(as_int);
            } else if (v.type.name == "string"){
                result = v.as_string();
            } else if (v.type.name == "bool"){
                auto as_bool = v.as_bool();
                result = as_bool ? "true" : "false";
            } else if (v.type == null->type){
                result = "null";
            } else {
                // Error!
                throw 1;
            }
        } else if (v.type.kind == ListType) {
            result += "[";
            auto values_in_v = v.as_list_value();
            for (auto val = values_in_v.begin(); val < values_in_v.end(); val++) {
                result += value_to_string(**val);

                if (val != values_in_v.end()-1) {
                    result += ", ";
                }
            }
            result += "]";
        } else {
            result = "<value> at: " + std::to_string(v.address);
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

    Value* Interpreter::visit_Double(Lexing::Token t) {
        return create_literal(t.value, "double");
    }

    Value* Interpreter::visit_Int(Lexing::Token t) {
        return create_literal(t.value, "int");
    }

    Value* Interpreter::visit_Bool(Lexing::Token t) {
        return create_literal(t.value, "bool");
    }

    Value* Interpreter::visit_Str(Lexing::Token t) {
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

    Value* Interpreter::visit_VarDeclaration(const Lexing::Token& var_type, Lexing::Token name, AST initial) {
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

    Value *Interpreter::visit_ListDeclaration(const Lexing::Token &var_type, Lexing::Token name, AST initial) {
        if (currentScope->symbolExists(name.value)) {
            // TODO: Handle Errors
            // NameError! Duplicate Variable name.value
            throw 1;
        }

        auto base_type = currentScope->findSymbol(var_type.value);
        if (!(base_type && base_type->isType)) {
            // TODO: Handle Errors
            // NameError! Invalid type: base_type.value
            throw 1;
        }

        Symbol* newVar;
        Value* valueReturn = null;

        Symbol* list_type;

        auto found_in_table = globalTable.findSymbol(base_type->name + "[]");
        if (found_in_table) {
            list_type = found_in_table;
        } else {
            list_type = globalTable.addSymbol({
                base_type, base_type->name + "[]",
                .isType=true,
                .kind=ListType
            });
        }

        if (initial.tp != NoOp) {
            auto newValue = visit(initial);

            newVar = currentScope->addSymbol({
                list_type,
                name.value,
                newValue
            });

            newValue->addReference(*newVar);
            newValue->kind = ListVal;
            valueReturn = newValue;

            return valueReturn;
        }

        currentScope->addSymbol({
            list_type,
            name.value
        });

        return valueReturn;
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

    Value* Interpreter::visit_Variable(Lexing::Token token) {
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

    Value* Interpreter::visit_Index(AST val, AST expr) {
        auto visited_val = visit(val);

        if (visited_val->type.name == "string") {
            auto str = visited_val->as_string();

            auto visited_indx = visit(expr);
            if (visited_indx->type.name == "int") {
                auto int_indx = visited_indx->as_int();

                if (int_indx >= 0 && int_indx < str.size()) {
                    std::string result(1, str[int_indx]);
                    return create_literal(result, "string");
                } else if (int_indx < 0 && abs(int_indx) <= str.size()) {
                    size_t actual_indx = str.size() - int_indx;
                    std::string result(1, str[actual_indx]);
                    return create_literal(result, "string");
                } else {
                    // TODO: Handle Error
                    // Error! Indexing string out of bounds
                    throw 1;
                }
            } else {
                // TODO: Handle Error
                // Error! Strings can only be indexed with integer values
                throw 1;
            }
        } else if (visited_val->kind == ListVal){
            auto list_value = visited_val->as_list_value();
            auto visited_indx = visit(expr);
            if (visited_indx->type.name == "int") {
                auto int_indx = visited_indx->as_int();

                if (int_indx >= 0 && int_indx < list_value.size()) {
                    return list_value[int_indx];
                } else if (int_indx < 0 && abs(int_indx) <= list_value.size()) {
                    size_t actual_indx = list_value.size() - int_indx;
                    return list_value[actual_indx];
                } else {
                    // TODO: Handle Error
                    // Error! Indexing string out of bounds
                    throw 1;
                }
            } else {
                // TODO: Handle Error
                // Error! Lists can only be indexed with integer values
                throw 1;
            }
        } else {
            // TODO: Handle error
            //Error! Index operator is only valid for string and lists values.
            throw 1;
        }

        return null;
    }

    Value *Interpreter::visit_ListExpression(std::vector<AST> elements) {
        Symbol* list_t = nullptr;
        std::vector<Symbol> list_syms;

        for (auto el : elements) {
            auto visited_element = visit(el);
            auto type_of_el = currentScope->findSymbol(visited_element->type.name);

            if (!list_t) {
                auto list_type_name = visited_element->type.name + "[]";

                // TODO: Fix. Apparently list types are stored only in the global scope.
                auto found_type = globalTable.findSymbol(list_type_name);
                if (found_type) {
                    list_t = found_type;
                } else {
                    list_t = globalTable.addListType(type_of_el);
                }
            }

            Value* actual_value;

            if (visited_element->type.kind == PrimitiveType) {
                actual_value = valueTable.copyValue(*visited_element);
            } else {
                actual_value = visited_element;
            }

            auto el_symbol = Symbol{
                type_of_el,
                "list_element",
                actual_value
            };

            actual_value->addReference(el_symbol);
            list_syms.push_back(el_symbol);
        }

        Symbol* list_type;
        if (list_t) {
            list_type = list_t;
        } else {
            list_type = globalTable.addListType(any_type);
        }

        auto in_val_table = valueTable.addNewValue(*list_type, list_syms);
        in_val_table->kind = ListVal;
        return in_val_table;
    }

    Value* Interpreter::visit_BinOp(Lexing::Token token, AST &left, AST &right) {
        auto leftVisited = visit(left);
        auto rightVisited = visit(right);

        switch (token.tp) {
            case Lexing::PLUS:
                if (leftVisited->kind == ListVal) {
                    Value* new_list;

                    if (rightVisited->kind == ListVal) {
                        std::vector<Symbol> new_elements;

                        for (auto el : leftVisited->as_list_symbol()) {
                            auto val = el.value;;

                            if (val->type.kind == PrimitiveType) {
                                val = valueTable.copyValue(*val);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }
                        for (auto el : rightVisited->as_list_symbol()) {
                            auto val = el.value;;

                            if (val->type.kind == PrimitiveType) {
                                val = valueTable.copyValue(*val);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }

                        new_list = valueTable.addNewValue(leftVisited->type, new_elements);
                        new_list->kind = ListVal;
                        return new_list;
                    } else {
                        std::vector<Symbol> new_elements;

                        for (auto el : leftVisited->as_list_symbol()) {
                            auto val = el.value;

                            if (val->type.kind == PrimitiveType) {
                                val = valueTable.copyValue(*val);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }

                        auto val = rightVisited;
                        if (val->type.kind == PrimitiveType) {
                            val = valueTable.copyValue(*val);
                        }

                        Symbol new_symbol = {currentScope->findSymbol(val->type.name), "list_element", val};
                        new_elements.push_back(new_symbol);
                        new_list = valueTable.addNewValue(*globalTable.addListType(&val->type), new_elements);
                        new_list->kind = ListVal;
                        return new_list;
                    }
                } else if (leftVisited->type.name == "string") {
                    return create_literal(leftVisited->as_string()+value_to_string(*rightVisited), "string");
                } else if (rightVisited->type.name == "string") {
                    return create_literal(value_to_string(*leftVisited) + rightVisited->as_string(), "string");
                } else if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type.name == "int") {
                        auto result = leftVisited->as_int() + rightVisited->as_int();
                        return create_literal(std::to_string(result), "int");
                    } else if (leftVisited->type.name == "double") {
                        auto result = leftVisited->as_double() + rightVisited->as_double();
                        return create_literal(std::to_string(result), "double");
                    }
                } else {
                    // TODO: Handle Error
                    // ValueError! Numeric adition must be with operands of the same type
                    throw 1;
                }
                break;
            case Lexing::MINUS:
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
            case Lexing::MUL:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type.name == "int") {
                        auto result = leftVisited->as_int() * rightVisited->as_int();
                        return create_literal(std::to_string(result), "int");
                    } else if (leftVisited->type.name == "double") {
                        auto result = leftVisited->as_double() * rightVisited->as_double();
                        return create_literal(std::to_string(result), "double");
                    }
                } else if (leftVisited->kind == ListVal && rightVisited->type.name == "int") {
                    int right_as_int = rightVisited->as_int();
                    std::vector<Symbol> new_elements;

                    for (int i = 0; i < right_as_int; i++) {
                        for (auto el : leftVisited->as_list_symbol()) {
                            auto val = el.value;

                            if (val->type.kind == PrimitiveType) {
                                val = valueTable.copyValue(*val);
                            }

                            Symbol new_symbol = {el.tp, el.name, val};
                            val->addReference(new_symbol);
                            new_elements.push_back(new_symbol);
                        }
                    }

                    auto new_list = valueTable.addNewValue(leftVisited->type, new_elements);
                    new_list->kind = ListVal;
                    return new_list;
                } else if (leftVisited->type.name == "string" && rightVisited->type.name == "int"){
                    int right_as_int = rightVisited->as_int();
                    std::string left_as_string = leftVisited->as_string();
                    std::string new_string;

                    for (int i = 0; i < right_as_int; i++) {
                        new_string += left_as_string;
                    }

                    auto new_val = create_literal(new_string, "string");
                    return new_val;
                } else {
                    // TODO: Handle Error
                    // ValueError! multiplication operation must be with operands of the same type
                    throw 1;
                }
                break;
            case Lexing::DIV:
                if (leftVisited->type == rightVisited->type && leftVisited->type.name == "double") {
                    auto result = leftVisited->as_double() / rightVisited->as_double();
                    return create_literal(std::to_string(result), "double");
                }else {
                    // TODO: Handle Error
                    // ValueError! Division operation must be with operands of type double
                    throw 1;
                }
                break;
            case Lexing::MOD:
                if (leftVisited->type == rightVisited->type && leftVisited->type.name == "int") {
                    auto result = leftVisited->as_int() % rightVisited->as_int();
                    return create_literal(std::to_string(result), "int");
                }else {
                    // TODO: Handle Error
                    // ValueError! Modulo operation must be with operands of type int
                    throw 1;
                }
                break;
            case Lexing::POW:
                if (leftVisited->type == rightVisited->type) {
                    if (leftVisited->type.name == "int") {
                        auto result = powl(leftVisited->as_int(), rightVisited->as_int());
                        return create_literal(std::to_string(result), "int");
                    } else if (leftVisited->type.name == "double") {
                        auto result = powl(leftVisited->as_double(), rightVisited->as_double());
                        return create_literal(std::to_string(result), "double");
                    }
                } else {
                    // TODO: Handle Error
                    // ValueError! Power operation must be with operands of the same type
                    throw 1;
                }
                break;
            case Lexing::EQU:
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
            case Lexing::NEQ:
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
            case Lexing::LT:
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
            case Lexing::GT:
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
            case Lexing::LET:
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
            case Lexing::GET:
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
            case Lexing::AND:
                if (leftVisited->type == rightVisited->type && leftVisited->type.name == "bool") {
                    auto result = leftVisited->as_bool() && rightVisited->as_bool();
                    return create_literal(result ? "true" : "false", "bool");
                }else {
                    // TODO: Handle Error
                    // ValueError! Division operation must be with operands of type double
                    throw 1;
                }
                break;
            case Lexing::OR:
                if (leftVisited->type == rightVisited->type && leftVisited->type.name == "bool") {
                    auto result = leftVisited->as_bool() || rightVisited->as_bool();
                    return create_literal(result ? "true" : "false", "bool");
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

    Value* Interpreter::visit_UnaryOp(Lexing::Token token, AST &left) {
        auto result = visit(left);

        switch (token.tp) {
            case Lexing::PLUS:
                break;
            case Lexing::MINUS:
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

        return null;
    }

    Value* Interpreter::visit_FuncExpression(std::vector<AST> params, const Lexing::Token& retType, AST body){
        auto returnType = retType.tp == Lexing::NOTHING ? nullptr : currentScope->findSymbol(retType.value);

        auto paramTypes = getParamTypes(params);

        auto typeName = Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(returnType, paramTypes);
        }

        auto funcValue = valueTable.addNewValue(*typeOfFunc, body);
        funcValue->scope = currentScope;
        funcValue->params = params;
        funcValue->kind = FunctionVal;

        return funcValue;
    }

    Value* Interpreter::visit_FuncDecl(const Lexing::Token& name, std::vector<AST> params, Lexing::Token retType, AST body){

        if (currentScope->symbolExists(name.value)) {
            // TODO: Handle Error
            // Error! Redefinition of symbol name.value
            throw 1;
        }

        auto returnType = retType.tp == Lexing::NOTHING ? nullptr : currentScope->findSymbol(retType.value);

        auto paramTypes = getParamTypes(params);

        auto typeName = Symbol::constructFuncTypeName(returnType, paramTypes);

        auto typeOfFunc = globalTable.findSymbol(typeName);

        if (!typeOfFunc) {
            typeOfFunc = globalTable.addFuncType(returnType, paramTypes);
        }

        auto funcValue = valueTable.addNewValue(*typeOfFunc, body);
        funcValue->scope = currentScope;
        funcValue->params = params;
        funcValue->kind = FunctionVal;

        auto funcSymbol = currentScope->addSymbol({
            typeOfFunc,
            name.value,
            funcValue,
            .kind=FunctionType
        });

        funcValue->addReference(*funcSymbol);

        return null;
    }

    Value *Interpreter::visit_FuncCall(AST expr, Lexing::Token fname, std::vector<AST> args) {
        if (callDepth >= MAX_CALL_DEPTH) {
            //TODO: Handle Error
            // Error! Max call depth exceeded
        }

        if (fname.tp != Lexing::NOTHING){
            auto found_in_natives = native_functions.find(fname.value);

            if (found_in_natives != native_functions.end()) {
                std::vector<Value*> arguments_visited;
                arguments_visited.reserve(args.size());

                for(auto arg : args){
                    arguments_visited.push_back(visit(arg));
                }

                return found_in_natives->second(arguments_visited);
            }
        }

        if (auto fVal = visit(expr);
            fVal->kind == FunctionVal) {

            auto funcScope = SymbolTable("func-scope", {}, fVal->scope);
            auto calleeScope = currentScope;

            std::vector<AST> newDecls;
            std::vector< std::pair<Lexing::Token, Value*> > initValues;

            for (int i = 0; i < fVal->params.size(); i++) {
                auto par = fVal->params[i];
                if (args.size() > i) {
                    switch (par.tp) {
                        case Declaration:
                        case ListDeclaration:
                        {
                            auto newValue = visit(args[i]);
                            if (newValue->type.kind == PrimitiveType) {
                                newValue = valueTable.copyValue(*newValue);
                            }
                            initValues.push_back({par.token, newValue});
                            break;
                        }
                        default:
                            break;
                    }
                }

                newDecls.push_back(par);
            }

            currentScope = &funcScope;
            callDepth++;

            for (int i = 0; i < newDecls.size(); i++) {
                visit(newDecls[i]);

                if (i < initValues.size()) {
                    auto newVar = currentScope->findSymbol(initValues[i].first.value);
                    newVar->value = initValues[i].second;
                    initValues[i].second->addReference(*newVar);
                }
            }

            auto body_as_ast = std::any_cast<AST>(fVal->val);

            auto result = visit(body_as_ast);
            currentScope = calleeScope;
            valueTable.cleanUp(funcScope);

            result->important = false;
            callDepth--;
            return result;
        }

        throw 1;
    }

    Value *Interpreter::visit_FuncBody(std::vector<AST> statements) {
        auto bodyScope = SymbolTable("func-body-scope", {}, currentScope);

        currentScope = &bodyScope;

        for (auto st : statements) {
            visit(st);
            if (returning) {
                break;
            }
        }

        valueTable.cleanUp(bodyScope);
        currentScope = bodyScope.getParent();

        auto ret = returning;
        returning = nullptr;
        return ret ? ret : null;
    }

    Value* Interpreter::visit_Return(AST val) {
        returning = visit(val);
        returning->important = true;
        return null;
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

    void Interpreter::interpret(std::string code) {
        parser.set_text(code);

        auto root = parser.program();

        visit(root);
    }

    Value* Interpreter::eval(std::string code) {
        parser.set_text(code);

        auto statements = parser.program_content();

    //    try{
        currentScope = &replScope;

        Value* result = null;
        for (auto node : statements) {
            result = visit(node);
        }

        currentScope = &globalTable;
    //    }

        return result;
    }

    std::vector<std::pair<Symbol, bool>> Interpreter::getParamTypes(const std::vector<AST>& params) {
        std::vector<std::pair<Symbol, bool>> ts;

        for (auto par : params) {
            switch (par.tp) {
                case Declaration:
                    if (auto ft = currentScope->findSymbol(par.type.value)) {
                        ts.emplace_back(*ft, par.nodes["initial"].tp == NoOp);
                    } else {
                        // TODO: Handle Error
                        // Error! Unknown type par.type.value
                        throw 1;
                    }
                    break;
                case ListDeclaration:
                    // FIXME: List types are registered as their basetype and not as listtype
                    if (auto ft = currentScope->findSymbol(par.type.value)) {
                        ts.emplace_back(*ft, par.nodes["initial"].tp == NoOp);
                    } else {
                        // TODO: Handle Error
                        // Error! Unknown type par.type.value
                    }
                    break;
                default:
                    // Error! Expected variable declaration inside function parenthesis.
                    break;
            }
        }

        return ts;
    }
}