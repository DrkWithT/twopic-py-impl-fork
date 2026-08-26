#ifndef LEXICAL_HPP
#define LEXICAL_HPP

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cstddef>

#include "frontend/token.hpp"

/* It turns a stream of raw characters into a stream of meaningful words
Lexer (Lexical Analysis): Checking spelling. (Is "appl" a word? No. Is "apple" a word? Yes.)
*/

namespace TwoPy::Frontend {
    std::string read_file(std::string_view filename);
    class lexical_class {
        private:
            std::string m_source {};
            std::size_t m_line {};
            std::size_t m_column {};
            const char* m_curr_pos {};
            const char* m_end {};

            std::unordered_map<std::string, token_type> predefined_keyword {};
            std::vector<std::size_t> indent {};
            void next_token();
             // Variable, Classes, etc etc names  
            void handle_indentation(std::vector<token_class>& tokens, std::size_t start_line);

            bool is_string() const;
            bool is_float() const;
            bool is_integer() const;
            bool is_whitespace() const;
            bool is_identifier() const;

        public:
            lexical_class(const std::string& source);

            /* tokenize the input strings */
            std::vector<token_class> tokenize();
    };
}

#endif