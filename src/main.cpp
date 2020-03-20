#include <iostream>
#include "Lexer/lexer.hpp"
#include "Parser/parser.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {

    std::string co = "int a = 100;";

    co = R"pow(
    class Node {
        string tp
        Node children[]
        init(string _tp, Node _children[]) {
            tp = _tp
            children = _children
        }
    }

    class Token {
        string tp

        init (string _tp) {
            tp = _tp
        }

        func toString(): string {
            return "Token(" + tp + ")"
        }
    }

    func contains(string lst[], string el): bool{
        for (int i = 0; i < length(lst); i++) {
            if lst[i] == el {
                return true
            }
        }

        return false
    }

    class Lexer {
        string text
        int pos

        static string operators[] = ["+", "-", ">", "<", ".", ",", "[", "]"]
        init(string _txt = "") {
            text = _txt
            pos = 0
        }

        func currChar(): string {
            if pos >= length(text) {
                return null
            }

            return text[pos]
        }

        func ignoreAny() {
            while currChar() != null && !contains(Lexer::operators, currChar()) {
                # println(currChar())
                pos++
            }
        }

        func getNextToken(): Node {
            ignoreAny()
            if (currChar() == null || pos >= length(text)) {
                return new Token(null)
            }

            if (currChar() == "+") {
                pos++
                return new Token("add")
            } else if (currChar() == "-") {
                pos++
                return new Token("sub")
            } else if (currChar() == ">") {
                pos++
                return new Token("pta")
            } else if (currChar() == "<") {
                pos++
                return new Token("pts")
            } else if (currChar() == "[") {
                pos++
                return new Token("opl")
            } else if (currChar() == "]") {
                pos++
                return new Token("cll")
            } else if (currChar() == ".") {
                pos++
                return new Token("dot")
            } else if (currChar() == ",") {
                pos++
                return new Token("com")
            }

            return new Token(null)
        }

        func setText(string _text) {
            text = _text
            pos = 0
        }
    }

    class AST {
        string tp
        AST children[]
        init (string _tp, AST _children[] = []) {
            tp = _tp
            children = _children
        }
    }

    class Parser {
        Lexer lex
        Token currToken
        init (Lexer _lex = new Lexer()) {
            lex = _lex
            currToken = new Token(null)
        }

        func eat(string tp) {
            if currToken.tp == tp {
                currToken = lex.getNextToken()
            } else {
                println("Error! Expected *", tp, "* but got *", currToken.tp, "*.")
                invalid_token = null
            }
        }

        func program(string text) {
            lex.setText(text)
            currToken = lex.getNextToken()
            AST result = block()

            return result
        }

        func block() {
            string delimiters[] = ["cll", null]
            AST statements[] = []
            while !contains(delimiters, currToken.tp) {
                AST n = statement()
                statements += n
            }

            return new AST("block", statements)
        }

        func statement() {
            if currToken.tp == "add" {
                eat("add")
                return new AST("add")
            } else if currToken.tp == "sub" {
                eat("sub")
                return new AST("sub")
            } else if currToken.tp == "pta" {
                eat("pta")
                return new AST("pta")
            } else if currToken.tp == "pts" {
                eat("pts")
                return new AST("pts")
            } else if currToken.tp == "dot" {
                eat("dot")
                return new AST("dot")
            } else if currToken.tp == "com" {
                eat("com")
                return new AST("com")
            } else if currToken.tp == "opl" {
                eat("opl")
                AST body[] = block()
                eat("cll")
                return new AST("opl", block())
            }
        }
    }

    class Interpreter {
        int pointer = 0
        int memory[]
        init() {
            memory = [0]*300
        }

        func visit(AST node) {
            if node.tp == "block" {
                for(int i = 0; i < length(node.children); i++) {
                    visit(node.children[i])
                }
            } else if node.tp == "add" {
                memory[pointer]++
                return null
            } else if node.tp == "sub" {
                memory[pointer]--
                return null
            }  else if node.tp == "pta" {
                pointer++
                return null
            } else if node.tp == "pts" {
                pointer--
                return null
            } else if node.tp == "dot" {
                println(".")
                return null
            } else if node.tp == "com" {
                println(",")
                return null
            } else if node.tp == "opl" {
                while(memory[pointer] > 0){
                    visit(node.children)
                }
            }
        }

        func interpret(string code) {
            Parser p = new Parser()
            AST tree = p.program(code)
            visit(tree)
        }
    }

    string co = "+++++[>>+<<-]"

    Interpreter p = new Interpreter()

    p.interpret(co)

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 30; j++) {
            print(p.memory[j + (30*i)])
        }
        println()
    }

    )pow";

    if (argc > 1) {
        co = argv[1];
    }

    Lexer a(co);

    Token cur = a.getNextToken();

    while (cur.tp != EOFT) {
        std::cout << "New Token: " << cur.value << "\n";
        cur = a.getNextToken();
    }

    Lexer actual_lex("");
    Parser actual_parser(actual_lex);

    actual_parser.set_text(co);
    auto result = actual_parser.program();

    std::cout << result.tp << "\n";

    // If there's a file to be read

        // Opening file and reading contents:
        // text lines.

        // Handle potential errors
            // Interpreting the text inside the file.

    // Else
        // Show an interactive prompt

        // Handle potential errors
            // Evaluate the input

            // Show the result

    return 0;
}
