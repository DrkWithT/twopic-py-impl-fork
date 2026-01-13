#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <string_view>
#include <cstddef>

#include "frontend/token.hpp"

/* In the future I'm hoping to get std::variant for AST to get more advanced grammar rules  */
namespace Ast {
    enum class node_type {
        PROGRAM,
        MODULE,

        EXPRESSION_STMT,
        ASSIGNMENT,
        AUGMENTED_ASSIGNMENT,
        RETURN_STMT,
        BREAK_STMT,
        CONTINUE_STMT,
        PASS_STMT,
        DEL_STMT,
        YIELD_STMT,
        RAISE_STMT,
        ASSERT_STMT,
        IMPORT_STMT,
        FROM_IMPORT_STMT,
        GLOBAL_STMT,
        NONLOCAL_STMT,

        IF_STMT,
        ELIF_STMT,
        ELSE_STMT,
        WHILE_STMT,
        FOR_STMT,
        TRY_STMT,
        EXCEPT_STMT,
        FINALLY_STMT,
        WITH_STMT,
        MATCH_STMT,
        CASE_STMT,

        FUNCTION_DEF,
        CLASS_DEF,
        SELF,
        LAMBDA_DEF,
        PARAMETER,
        PARAMETER_LIST,
        PARAMETER_DICT,

        BINARY_OP,
        UNARY_OP,
        COMPARISON,
        LOGICAL_OP,
        CALL_EXPR,
        ATTRIBUTE_EXPR,
        SUBSCRIPT_EXPR,

        INTEGER_LITERAL,
        FLOAT_LITERAL,
        STRING_LITERAL,
        BYTES_LITERAL,
        TRUE_LITERAL,
        FALSE_LITERAL,
        NONE_LITERAL,

        IDENTIFIER,
        BLOCK,
        ARGUMENT_LIST,
        TUPLE,
        LIST,
        DICT,
        SET
    };

    struct ast_node {
        node_type type {};
        Token::token_class token_m{Token::token_type::DEFAULT, "", 0, 0};
        std::vector<std::unique_ptr<ast_node>> children_m {};

        void add_child(std::unique_ptr<ast_node> child) {
            children_m.push_back(std::move(child));
        }
    };

    class ast_class {
        private:
            std::unique_ptr<ast_node> root = nullptr;

        public:
            void set_root(std::unique_ptr<ast_node> node) {
                root = std::move(node);
            }

            ast_node* get_root() const {
                return root.get();
            }

            void clear() {
                root.reset();
            }

            bool is_empty() const {
                return root == nullptr;
            }
    };
}

#endif