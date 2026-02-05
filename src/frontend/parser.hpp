#ifndef PARSER_HPP
#define PARSER_HPP

#include <utility>
#include <string>
#include <format>
#include <iostream>
#include <concepts>

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
            /* Should make is a vector of bools */
            bool valid_constructor = false;
            
            std::vector<Token::token_class> tokens {};
            std::size_t current_pos {};
            std::size_t m_previous_pos {};
            
            Token::token_class& current_token();
            Token::token_class& previous_token();

            int m_error_count {};

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
                    m_previous_pos = current_pos;
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

                m_previous_pos = current_pos;
                current_pos++;
            }

            // Something that produces a value
            Ast::ExprPtr parse_expression_types();
            // Performs a action ex: if, while, for, return
            Ast::StmtPtr parse_statement();

            Ast::StmtPtr parse_function_def();
            Ast::StmtPtr parse_class();
            Ast::StmtPtr parse_if_stmt();
            Ast::StmtPtr parse_while_stmt();
            Ast::StmtPtr parse_for_stmt();
            Ast::StmtPtr parse_match_stmt();
            Ast::StmtPtr parse_return_stmt();
            Ast::StmtPtr parse_pass();
            Ast::StmtPtr parse_try();
            Ast::StmtPtr parse_break();
            Ast::StmtPtr parse_continue();
            Ast::StmtPtr parse_method();
            Ast::StmtPtr parse_lambda();
            Ast::StmtPtr parse_expression_stmt(const auto& token);

            Ast::ExprPtr parse_list();
            Ast::ExprPtr parse_dict();

            Ast::ExprPtr parse_call_expr(Ast::ExprPtr callee);
            Ast::ExprPtr parse_constructor_call(Ast::ExprPtr constructor);
            Ast::ExprPtr parse_attribute_expr();
            Ast::ExprPtr parse_self();

            Ast::ExprPtr parse_term();
            Ast::ExprPtr parse_factor();
            Ast::ExprPtr parse_power();
            Ast::StmtPtr parse_case();
            Ast::ExprPtr parse_equality();
            Ast::ExprPtr parse_comparator();
            Ast::ExprPtr parse_assignment();
            Ast::ExprPtr parse_bitwise();
            Ast::ExprPtr parse_logical_or();
            Ast::ExprPtr parse_logical_and();

            Ast::Block parse_block();

            void consume_newline();
            void consume_line();

        public:
            parser_class(Lexical::lexical_class& lexer);

            /* Parses the file */
            Ast::Program parse();
    };
}

#endif
