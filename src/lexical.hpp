#ifndef LEXICAL_HPP
#define LEXICAL_HPP 

#include <string>
#include <string_view>
#include <unordered_map>
#include <fstream>
#include <vector>

#include "token.hpp"

/* It turns a stream of raw characters into a stream of meaningful words 
Lexer (Lexical Analysis): Checking spelling. (Is "appl" a word? No. Is "apple" a word? Yes.)
*/

namespace Lexical {
    class lexical_class {
        private:
            size_t position;
            std::string_view source;
            size_t line;
            size_t column;

            std::unordered_map<std::string, Token::token_type> keyword;

            void next_token();  

            bool is_indentation();
            bool is_string();
            bool is_float();
            bool is_integer();
            bool is_whitespace();
            bool is_varaible();
            bool is_function();

        public:
            lexical_class(std::string_view source);

            /* tokenize the input strings */
            std::vector<Token::token_class> tokenize();

            std::string token_type_name(Token::token_class type);
    };
}

#endif