#ifndef PARSER_HPP
#define PARSER_HPP 

#include <utility>
#include <string>
#include <format>
#include <iostream>

#include <fmt/core.h>

#include "frontend/ast.hpp"
#include "frontend/lexical.hpp"

/* Basically anylisis the syntax of a group of phrases */

/* Old notes sucks but its basically a top-down or LL parser*/


/* 
Left-Associative example:
a - b - c  ≡  (a - b) - c
Expr → Term { op Term }

Right-Associative example:
a ** b ** c  ≡  a ** (b ** c)
Expr → Base [ op Expr ]   (uses recursion, not loop)
*/


namespace Parser {
    /* I decided upon a recursive descent appoarch */    
    class parser_class {
        private:
            std::vector<Token::token_class> tokens {};
            std::size_t current_pos {};
            Ast::ast_class ast_tree {};
            Token::token_class& current_token();
            int m_error_count;

            bool match(const Token::token_type& type);

            // Might be useful to track the amount of errors
            void debug_syntax_error() {
                m_error_count++;
                throw std::runtime_error {
                    fmt::format("Syntax Error at: line {} column {} ", std::to_string(current_token().line), std::to_string(current_token().column)) 
                };
            }

            // Special thanks to DerkT for fixing up my code!
            template <typename TokenType, typename ... Rest> requires (std::same_as<TokenType, Token::token_type>)
            bool match(TokenType first_type, Rest ... more_types) noexcept {
                const auto current_tag = tokens.at(current_pos).type;
                return ((current_tag == first_type) || ... || (current_tag == more_types));
            }

            bool is_at_end();

            // Special thanks to DerkT for fixing up my code!
            template <typename ... TokenTypes>
            void consume(TokenTypes ... types) {
                if constexpr (sizeof...(types) < 1) {
                    current_pos++;
                    return;
                } else {
                    // Basically if i consumed the wrong type of the current type like consume(Colon) != match(Newline)
                    // it has a runtime error
                    if (const auto& current_token_ref = tokens.at(current_pos); !match(types...)) {
                        throw std::runtime_error(
                            std::format(
                                "Parse Error at source:{}:{}: Unexpected token.\n",
                                current_token_ref.line,
                                current_token_ref.column
                            )
                        );
                    }
                }

                current_pos++;
            }

             // Something that produces a value 
            std::unique_ptr<Ast::ast_node> parse_expression_types();
            // Performs a action ex: if, while, for, return 
            std::unique_ptr<Ast::ast_node> parse_statement();

            std::unique_ptr<Ast::ast_node> parse_function_def();
            std::unique_ptr<Ast::ast_node> parse_class();
            std::unique_ptr<Ast::ast_node> parse_if_stmt();
            std::unique_ptr<Ast::ast_node> parse_while_stmt();
            std::unique_ptr<Ast::ast_node> parse_for_stmt();
            std::unique_ptr<Ast::ast_node> parse_match_stmt();
            std::unique_ptr<Ast::ast_node> parse_return_stmt();
            std::unique_ptr<Ast::ast_node> parse_pass();
            std::unique_ptr<Ast::ast_node> parse_try();
            std::unique_ptr<Ast::ast_node> parse_break();
            std::unique_ptr<Ast::ast_node> parse_continue(); 
            std::unique_ptr<Ast::ast_node> parse_method();
            std::unique_ptr<Ast::ast_node> parse_lambda();

            std::unique_ptr<Ast::ast_node> parse_list();
            std::unique_ptr<Ast::ast_node> parse_dict();

            std::unique_ptr<Ast::ast_node> parse_call_expr();
            std::unique_ptr<Ast::ast_node> parse_attribute_expr();
            std::unique_ptr<Ast::ast_node> parse_self();

            std::unique_ptr<Ast::ast_node> parse_term();
            std::unique_ptr<Ast::ast_node> parse_factor();
            std::unique_ptr<Ast::ast_node> parse_power();
            std::unique_ptr<Ast::ast_node> parse_case();
            std::unique_ptr<Ast::ast_node> parse_equality();
            std::unique_ptr<Ast::ast_node> parse_comparator();
            std::unique_ptr<Ast::ast_node> parse_assignment();
            std::unique_ptr<Ast::ast_node> parse_bitwise();

            void consume_newline();
            void consume_line();

        public:
            parser_class(Lexical::lexical_class& lexer);

            /* Parses the file */
            std::unique_ptr<Ast::ast_node> parse();
            Ast::ast_class& get_ast();
    };
}

#endif