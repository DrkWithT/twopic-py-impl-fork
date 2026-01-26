#ifndef PARSER_HPP
#define PARSER_HPP 

#include <utility>

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
            Token::token_class& previous_token();

            bool match(const Token::token_type& type);
            bool is_at_end();            

            bool consume(const Token::token_type& type);

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
            std::unique_ptr<Ast::ast_node> parse_list();
            std::unique_ptr<Ast::ast_node> parse_method();
            std::unique_ptr<Ast::ast_node> parse_dict();
            std::unique_ptr<Ast::ast_node> parse_lambda();

            std::unique_ptr<Ast::ast_node> parse_term();
            std::unique_ptr<Ast::ast_node> parse_factor();
            std::unique_ptr<Ast::ast_node> parse_power();
            std::unique_ptr<Ast::ast_node> parse_case();
            std::unique_ptr<Ast::ast_node> parse_equality();
            std::unique_ptr<Ast::ast_node> parse_comparator();
            std::unique_ptr<Ast::ast_node> parse_assignment();
            std::unique_ptr<Ast::ast_node> parse_bitwise();

            std::unique_ptr<Ast::ast_node> parse_misc_expression(std::unique_ptr<Ast::ast_node> node);

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