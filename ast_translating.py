
ast_node_names = [
    "Double",
    "Int",
    "Bool",
    "Str",
    "TernaryOp",
    "BinOp",
    "UnaryOp",
    "NoOp",
    "VarDeclaration",
    "ListDeclaration",
    "Variable",
    "Assignment",
    "ListExpression",
    "Block",
    "FuncExpression",
    "FuncDecl",
    "FuncCall",
    "FuncBody",
    "Return",
    "If",
    "For",
    "ForEach",
    "FoRange",
    "While",
    "Loop",
    "Break",
    "Continue",
    "Null",
    "Debug",
    "Module",
    "Import",
    "Enum",
    "Class",
    "ClassBody",
    "InstanceBody",
    "ClassInitializer",
    "ConstructorDecl",
    "ConstructorCall",
    "StaticStatement",
    "MemberVar",
    "StaticVar",
    "Index",
]

node_members = {
    "Double": [
        "Lexing::Token token"
    ],
    "Int": [
        "Lexing::Token token"
    ],
    "Bool": [
        "Lexing::Token token"
    ],
    "Str": [
        "Lexing::Token token"
    ],
    "Block": [
        "std::vector<std::shared_ptr<Parsing::Node>> statements"
    ],
    "BinOp": [
        "Lexing::Token token",
        "std::shared_ptr<Parsing::Node> left",
        "std::shared_ptr<Parsing::Node> right"
    ],
    "UnaryOp": [
        "Lexing::Token token",
        "std::shared_ptr<Parsing::Node> ast"
    ],
    "TernaryOp": [
        "std::shared_ptr<Parsing::Node> cond",
        "std::shared_ptr<Parsing::Node> trueb",
        "std::shared_ptr<Parsing::Node> falseb"
    ],
    "If": [
        "std::shared_ptr<Parsing::Node> cond",
        "std::shared_ptr<Parsing::Node> trueb",
        "std::shared_ptr<Parsing::Node> falseb"
    ],
    "For": [
        "std::shared_ptr<Parsing::Node> ini",
        "std::shared_ptr<Parsing::Node> cond",
        "std::shared_ptr<Parsing::Node> incr",
        "std::shared_ptr<Parsing::Node> body"
    ],
    "ForEach": [
        "Lexing::Token var",
        "std::shared_ptr<Parsing::Node> lst",
        "std::shared_ptr<Parsing::Node> body",
        "Lexing::Token rev"
    ],
    "FoRange": [
        "Lexing::Token var",
        "std::shared_ptr<Parsing::Node> first",
        "std::shared_ptr<Parsing::Node> second",
        "std::shared_ptr<Parsing::Node> body",
        "Lexing::Token rev"
    ],
    "While": [
        "std::shared_ptr<Parsing::Node> cond",
        "std::shared_ptr<Parsing::Node> body"
    ],
    "Loop": [
        "std::shared_ptr<Parsing::Node> body"
    ],
    "VarDeclaration": [
        "Lexing::Token var_type",
        "Lexing::Token name",
        "std::shared_ptr<Parsing::Node> initial"
    ],
    "ListDeclaration": [
        "Lexing::Token var_type",
        "Lexing::Token name",
        "std::shared_ptr<Parsing::Node> initial"
    ],
    "Assignment": [
        "std::shared_ptr<Parsing::Node> expr",
        "std::shared_ptr<Parsing::Node> val"
    ],
    "Variable": [
        "Lexing::Token token"
    ],
    "Index": [
        "std::shared_ptr<Parsing::Node> val",
        "std::shared_ptr<Parsing::Node> expr"
    ],
    "ListExpression": [
        "std::vector<std::shared_ptr<Parsing::Node>> elements"
    ],
    "Module": [
        "Lexing::Token name",
        "std::vector<std::shared_ptr<Parsing::Node>> statements"
    ],
    "Import": [
        "Lexing::Token path",
        "Lexing::Token name"
    ],
    "FuncExpression": [
        "std::vector<std::shared_ptr<Parsing::Node>> params",
        "Lexing::Token retType",
        "std::shared_ptr<Parsing::Node> body"
    ],
    "FuncDecl": [
        "Lexing::Token name",
        "std::vector<std::shared_ptr<Parsing::Node>> params",
        "Lexing::Token retType",
        "std::shared_ptr<Parsing::Node> body"
    ],
    "FuncCall": [
        "std::shared_ptr<Parsing::Node> expr",
        "Lexing::Token fname",
        "std::vector<std::shared_ptr<Parsing::Node>> args"
    ],
    "FuncBody": [
        "std::vector<std::shared_ptr<Parsing::Node>> statements"
    ],
    "Return": [
        "std::shared_ptr<Parsing::Node> val"
    ],
    "Enum": [
        "Lexing::Token name",
        "std::vector<std::shared_ptr<Parsing::Node>> variants"
    ],
    "Class": [
        "Lexing::Token name",
        "Lexing::Token ty",
        "std::shared_ptr<Parsing::Node> body"
    ],
    "ClassBody": [
        "std::vector<std::shared_ptr<Parsing::Node>> statements"
    ],
    "ConstructorDecl": [
        "std::vector<std::shared_ptr<Parsing::Node>> params",
        "std::shared_ptr<Parsing::Node> body"
    ],
    "ConstructorCall": [
        "Lexing::Token t"
    ],
    "ClassInitializer": [
        "Lexing::Token name",
        "std::vector<std::shared_ptr<Parsing::Node>> params"
    ],
    "InstanceBody": [
        "std::vector<std::shared_ptr<Parsing::Node>> statements"
    ],
    "MemberVar": [
        "std::shared_ptr<Parsing::Node> inst",
        "Lexing::Token name"
    ],
    "StaticStatement": [
        "std::shared_ptr<Node> statement"
    ],
    "StaticVar": [
        "std::shared_ptr<Parsing::Node> inst",
        "Lexing::Token name"
    ],
}

ast_header_file_template = """
#include "Parser/AST/Node.h"

namespace Odo::Parsing {
struct <node-name> : public Node {
<node-members>
    NodeType kind() final { return NodeType::<node-type>; }

    <node-constructor><header-constr-termn>
};
}
"""

ast_impl_file_template = """
#include "Parser/AST/<node-name>.h"

namespace Odo::Parsing {

<node-name>::<node-constructor>
<constructor-init>{}

}

"""

constructor_template = "<node-name>(<constructor-params>)"

def translate(st, table):
    result = st
    for k, v in table.items():
        result = result.replace("<"+k+">", v)

    return result

include_template = "include/Parser/AST/<node-name>.h"
src_template = "src/Parser/AST/<node-name>.cpp"

include_files = []
src_files = []

for node_type_name in ast_node_names:
    translation_dict = {"node-type":node_type_name}

    node_name = node_type_name + "Node"
    translation_dict["node-name"] = node_name

    members = node_members.get(node_type_name)
    has_impl = members is not None

    indentation = "    "
    node_member_definitions = ""
    constructor_init = ""
    constructor_params = ""

    if has_impl:
        constructor_init = indentation + ":"
        node_member_definitions = indentation
        for i, mem in enumerate(members):
            node_member_definitions += mem + ";\n" + indentation
            constructor_params += mem + "_p"

            mem_name = mem[mem.rfind(" ")+1:]
            constructor_init += " " + mem_name + "(" + mem_name + "_p" + ")"
            if i != len(members)-1:
                constructor_params += ", "
                constructor_init += "\n" + indentation + ","

    translation_dict["constructor-params"] = constructor_params
    translation_dict["constructor-init"] = constructor_init
    translation_dict["node-members"] = node_member_definitions

    node_constructor = translate(constructor_template, translation_dict)
    translation_dict["node-constructor"] = node_constructor
    translation_dict["header-constr-termn"] = ";" if has_impl else "{}"

    this_nodes_header = translate(ast_header_file_template, translation_dict)

    # print("===================================")
    # print(this_nodes_header)

    header_file_name = translate(include_template, translation_dict)
    include_files.append(header_file_name)

    with open(header_file_name, 'w') as f:
        f.write(this_nodes_header)

    if has_impl:
        this_nodes_impl = translate(ast_impl_file_template, translation_dict)

        # print(this_nodes_impl)
        impl_file_name = translate(src_template, translation_dict)
        src_files.append(impl_file_name)

        with open(impl_file_name, 'w') as f:
            f.write(this_nodes_impl)

for f in include_files:
    print("#include \"" + f[f.find("/")+1:] + "\"")

# print()
# for f in src_files:
#     print(f)
