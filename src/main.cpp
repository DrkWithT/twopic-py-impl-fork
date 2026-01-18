#include <fstream>
#include <sstream>
#include <string_view>
#include <string>
#include <fmt/core.h>

#include "frontend/lexical.hpp"
#include "frontend/parser.hpp"

void print_ast(const Ast::ast_node* node, int indent = 0) {
    if (!node) return;

    for (int i = 0; i < indent; i++) {
        fmt::print("  ");
    }

    const char* type_str = "UNKNOWN";
    switch (node->type) {
        case Ast::node_type::EQUALITY_OP: type_str = "EQUALITY_OP"; break;
        case Ast::node_type::PROGRAM: type_str = "PROGRAM"; break;
        case Ast::node_type::TRY_STMT: type_str = "TRY_STMT"; break;
        case Ast::node_type::EXCEPT_STMT: type_str = "EXCEPT_STMT"; break;
        case Ast::node_type::FINALLY_STMT: type_str = "FINALLY_STMT"; break;
        case Ast::node_type::FUNCTION_DEF: type_str = "FUNCTION_DEF"; break;
        case Ast::node_type::CLASS_DEF: type_str = "CLASS_DEF"; break;
        case Ast::node_type::IF_STMT: type_str = "IF_STMT"; break;
        case Ast::node_type::ELIF_STMT: type_str = "ELIF_STMT"; break;
        case Ast::node_type::ELSE_STMT: type_str = "ELSE_STMT"; break;
        case Ast::node_type::WHILE_STMT: type_str = "WHILE_STMT"; break;
        case Ast::node_type::FOR_STMT: type_str = "FOR_STMT"; break;
        case Ast::node_type::MATCH_STMT: type_str = "MATCH_STMT"; break;
        case Ast::node_type::CASE_STMT: type_str = "CASE_STMT"; break;
        case Ast::node_type::RETURN_STMT: type_str = "RETURN_STMT"; break;
        case Ast::node_type::BREAK_STMT: type_str = "BREAK_STMT"; break;
        case Ast::node_type::CONTINUE_STMT: type_str = "CONTINUE_STMT"; break;
        case Ast::node_type::PASS_STMT: type_str = "PASS_STMT"; break;
        case Ast::node_type::ASSIGNMENT: type_str = "ASSIGNMENT"; break;
        case Ast::node_type::AUGMENTED_ASSIGNMENT: type_str = "AUGMENTED_ASSIGNMENT"; break;
        case Ast::node_type::EXPRESSION_STMT: type_str = "EXPRESSION_STMT"; break;
        case Ast::node_type::BINARY_OP: type_str = "BINARY_OP"; break;
        case Ast::node_type::UNARY_OP: type_str = "UNARY_OP"; break;
        case Ast::node_type::LOGICAL_OP: type_str = "LOGICAL_OP"; break;
        case Ast::node_type::CALL_EXPR: type_str = "CALL_EXPR"; break;
        case Ast::node_type::ATTRIBUTE_EXPR: type_str = "ATTRIBUTE_EXPR"; break;
        case Ast::node_type::IDENTIFIER: type_str = "IDENTIFIER"; break;
        case Ast::node_type::INTEGER_LITERAL: type_str = "INTEGER_LITERAL"; break;
        case Ast::node_type::FLOAT_LITERAL: type_str = "FLOAT_LITERAL"; break;
        case Ast::node_type::STRING_LITERAL: type_str = "STRING_LITERAL"; break;
        case Ast::node_type::TRUE_LITERAL: type_str = "TRUE_LITERAL"; break;
        case Ast::node_type::FALSE_LITERAL: type_str = "FALSE_LITERAL"; break;
        case Ast::node_type::NONE_LITERAL: type_str = "NONE_LITERAL"; break;
        case Ast::node_type::PARAMETER_LIST: type_str = "PARAMETER_LIST"; break;
        case Ast::node_type::PARAMETER: type_str = "PARAMETER"; break;
        case Ast::node_type::COMPARISON: type_str = "COMPARISON"; break;
        case Ast::node_type::ARGUMENT_LIST: type_str = "ARGUMENT_LIST"; break;
        case Ast::node_type::BLOCK: type_str = "BLOCK"; break;
        case Ast::node_type::LIST: type_str = "LIST"; break;
        case Ast::node_type::DICT: type_str = "DICT"; break;
        default: break;
    }

    fmt::print("Node: {}", type_str);

    if (!node->token_m.value.empty()) {
        fmt::print(" (value: \"{}\")", node->token_m.value);
    }

    if (node->token_m.line > 0 || node->token_m.column > 0) {
        fmt::print(" [line {}, col {}]", node->token_m.line, node->token_m.column);
    }

    fmt::print("\n");

    for (const auto& child : node->children_m) {
        print_ast(child.get(), indent + 1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fmt::print(stderr, "Usage: {} <file.py>\n", argv[0]);
        fmt::print(stderr, "Example: {} test.py\n", argv[0]);
        return 1;
    }

    fmt::print("=== Parsing file: {} ===\n\n", argv[1]);

    try {
        std::string source_code = Lexical::read_file(argv[1]);

        Lexical::lexical_class lexer(source_code);

        Parser::parser_class parser(lexer);
        auto ast_root = parser.parse();

        fmt::print("\n=== ABSTRACT SYNTAX TREE ===\n");

        auto& ast_tree = parser.get_ast();
        if (!ast_tree.is_empty()) {
            print_ast(ast_tree.get_root());
        } else {
            fmt::print("(empty AST)\n");
        }

        fmt::print("\n=== PARSING COMPLETE ===\n");

    } catch (const std::exception& e) {
        fmt::print(stderr, "Error: {}\n", e.what());
        return 1;
    }

    return 0;
}
