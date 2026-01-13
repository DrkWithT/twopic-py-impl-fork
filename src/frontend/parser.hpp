#ifndef PARSER_HPP
#define PARSER_HPP 

#include "frontend/ast.hpp"
#include "frontend/lexical.hpp"

/* Basically anylisis the syntax of a group of phrases */


/* Left-associative would be loops 
   Right-associative would be recursion  */
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

            /* 
            Consume advances the token when match returns true and 
            allows for more error handling 
            */
            bool consume(const Token::token_type& type);
            bool is_at_end();            

             // Something that produces a value 
            std::unique_ptr<Ast::ast_node> parse_expression();
            // Performs a action ex: if, while, for, return 
            std::unique_ptr<Ast::ast_node> parse_statement();

            std::unique_ptr<Ast::ast_node> parse_function_def();
            std::unique_ptr<Ast::ast_node> parse_class();
            std::unique_ptr<Ast::ast_node> parse_if_stmt();
            std::unique_ptr<Ast::ast_node> parse_while_stmt();
            std::unique_ptr<Ast::ast_node> parse_for_stmt();
            std::unique_ptr<Ast::ast_node> parse_match_stmt();
            std::unique_ptr<Ast::ast_node> parse_return_stmt();
            std::unique_ptr<Ast::ast_node> parse_assignment();
            std::unique_ptr<Ast::ast_node> parse_operator();
            std::unique_ptr<Ast::ast_node> parse_case();
            std::unique_ptr<Ast::ast_node> parse_list();
            std::unique_ptr<Ast::ast_node> parse_pass();
            //std::unique_ptr<Ast::ast_node> parse_dict();

            void consume_newline();
            void consume_line();

        public:
            parser_class(Lexical::lexical_class& lexer) : current_pos(0) {};

            /* Parses the file */
            std::unique_ptr<Ast::ast_node> parse();
            Ast::ast_class& get_ast();
            void print();
    };
}

#endif