#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string_view>
#include <string>

#include "frontend/lexical.hpp"
#include "frontend/parser.hpp"

void print_ast(const Ast::ast_node* node, int indent = 0) {
    if (!node) return;

    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }

    std::cout << "Node: ";
    switch (node->type) {
        case Ast::node_type::PROGRAM: std::cout << "PROGRAM"; break;
        case Ast::node_type::TRY_STMT: std::cout << "TRY_STMT"; break;
        case Ast::node_type::EXCEPT_STMT: std::cout << "EXCEPT_STMT"; break;
        case Ast::node_type::FUNCTION_DEF: std::cout << "FUNCTION_DEF"; break;
        case Ast::node_type::CLASS_DEF: std::cout << "CLASS_DEF"; break;
        case Ast::node_type::IF_STMT: std::cout << "IF_STMT"; break;
        case Ast::node_type::ELIF_STMT: std::cout << "ELIF_STMT"; break;
        case Ast::node_type::ELSE_STMT: std::cout << "ELSE_STMT"; break;
        case Ast::node_type::WHILE_STMT: std::cout << "WHILE_STMT"; break;
        case Ast::node_type::FOR_STMT: std::cout << "FOR_STMT"; break;
        case Ast::node_type::MATCH_STMT: std::cout << "MATCH_STMT"; break;
        case Ast::node_type::CASE_STMT: std::cout << "CASE_STMT"; break;
        case Ast::node_type::RETURN_STMT: std::cout << "RETURN_STMT"; break;
        case Ast::node_type::BREAK_STMT: std::cout << "BREAK_STMT"; break;
        case Ast::node_type::CONTINUE_STMT: std::cout << "CONTINUE_STMT"; break;
        case Ast::node_type::PASS_STMT: std::cout << "PASS_STMT"; break;
        case Ast::node_type::ASSIGNMENT: std::cout << "ASSIGNMENT"; break;
        case Ast::node_type::AUGMENTED_ASSIGNMENT: std::cout << "AUGMENTED_ASSIGNMENT"; break;
        case Ast::node_type::EXPRESSION_STMT: std::cout << "EXPRESSION_STMT"; break;
        case Ast::node_type::BINARY_OP: std::cout << "BINARY_OP"; break;
        case Ast::node_type::UNARY_OP: std::cout << "UNARY_OP"; break;
        case Ast::node_type::LOGICAL_OP: std::cout << "LOGICAL_OP"; break;
        case Ast::node_type::CALL_EXPR: std::cout << "CALL_EXPR"; break;
        case Ast::node_type::ATTRIBUTE_EXPR: std::cout << "ATTRIBUTE_EXPR"; break;
        case Ast::node_type::IDENTIFIER: std::cout << "IDENTIFIER"; break;
        case Ast::node_type::INTEGER_LITERAL: std::cout << "INTEGER_LITERAL"; break;
        case Ast::node_type::FLOAT_LITERAL: std::cout << "FLOAT_LITERAL"; break;
        case Ast::node_type::STRING_LITERAL: std::cout << "STRING_LITERAL"; break;
        case Ast::node_type::TRUE_LITERAL: std::cout << "TRUE_LITERAL"; break;
        case Ast::node_type::FALSE_LITERAL: std::cout << "FALSE_LITERAL"; break;
        case Ast::node_type::NONE_LITERAL: std::cout << "NONE_LITERAL"; break;
        case Ast::node_type::PARAMETER_LIST: std::cout << "PARAMETER_LIST"; break;
        case Ast::node_type::PARAMETER: std::cout << "PARAMETER"; break;
        case Ast::node_type::ARGUMENT_LIST: std::cout << "ARGUMENT_LIST"; break;
        case Ast::node_type::BLOCK: std::cout << "BLOCK"; break;
        case Ast::node_type::LIST: std::cout << "LIST"; break;
        case Ast::node_type::DICT: std::cout << "DICT"; break;
        default: std::cout << "UNKNOWN"; break;
    }

    if (!node->token_m.value.empty()) {
        std::cout << " (value: \"" << node->token_m.value << "\")";
    }

    if (node->token_m.line > 0 || node->token_m.column > 0) {
        std::cout << " [line " << node->token_m.line << ", col " << node->token_m.column << "]";
    }

    std::cout << std::endl;

    for (const auto& child : node->children_m) {
        print_ast(child.get(), indent + 1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.py>" << std::endl;       std::cerr << "Example: " << argv[0] << " test.py" << std::endl;
        return 1;
    }

    std::cout << "=== Parsing file: " << argv[1] << " ===" << std::endl;
    std::cout << std::endl;

    try {
        std::string source_code = Lexical::read_file(argv[1]);

        Lexical::lexical_class lexer(source_code);

        Parser::parser_class parser(lexer);
        auto ast_root = parser.parse();

        std::cout << std::endl;
        std::cout << "=== ABSTRACT SYNTAX TREE ===" << std::endl;

        auto& ast_tree = parser.get_ast();
        if (!ast_tree.is_empty()) {
            print_ast(ast_tree.get_root());
        } else {
            std::cout << "(empty AST)" << std::endl;
        }

        std::cout << std::endl;
        std::cout << "=== PARSING COMPLETE ===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
