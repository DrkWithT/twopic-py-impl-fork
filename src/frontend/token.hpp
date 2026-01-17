#ifndef TOKEN_HPP
#define TOKEN_HPP 

#include <string>
#include <unordered_map>
#include <cstddef>

/* The token determines what type a certain phrase or word is going to be for a character*/

namespace Token {
    enum class token_type {
        KEYWORD_FALSE, KEYWORD_NONE, KEYWORD_TRUE,
        KEYWORD_AND, KEYWORD_AS, KEYWORD_ASSERT, KEYWORD_ASYNC, KEYWORD_AWAIT,
        KEYWORD_BREAK,
        KEYWORD_CLASS,
        KEYWORD_CONTINUE,
        KEYWORD_MATCH, KEYWORD_CASE,
        KEYWORD_DEF, KEYWORD_DEL,
        KEYWORD_ELIF, KEYWORD_ELSE, KEYWORD_ENUM, KEYWORD_EXCEPT,
        KEYWORD_FINALLY, KEYWORD_FOR, KEYWORD_FROM,
        KEYWORD_GLOBAL,
        KEYWORD_IF, KEYWORD_IMPORT, KEYWORD_IN, KEYWORD_IS,
        KEYWORD_LAMBDA,
        KEYWORD_NONLOCAL, KEYWORD_NOT,
        KEYWORD_OR,
        KEYWORD_PASS,
        KEYWORD_RAISE, KEYWORD_RETURN,
        KEYWORD_TRY,
        KEYWORD_WHILE, KEYWORD_WITH,
        KEYWORD_YIELD,
        KEYWORD_SELF,
        KEYWORD_INIT,

        LIST,

        IDENTIFIER,
        INTEGER_LITERAL,
        FLOAT_LITERAL,
        STRING_LITERAL,
        BYTES_LITERAL,

        PLUS,
        MINUS,
        STAR,
        SLASH,
        DOUBLE_SLASH,
        PERCENT,
        POWER,
        AT,

        AMPERSAND,
        PIPE,
        CARET,
        TILDE,
        LEFT_SHIFT,
        RIGHT_SHIFT,

        LESS,
        GREATER,
        LESS_EQUAL,
        GREATER_EQUAL,
        DOUBLE_EQUAL,
        NOT_EQUAL,

        EQUAL,
        PLUS_EQUAL,
        MINUS_EQUAL,
        STAR_EQUAL,
        SLASH_EQUAL,
        DOUBLE_SLASH_EQUAL,
        PERCENT_EQUAL,
        POWER_EQUAL,
        AT_EQUAL,
        AMPERSAND_EQUAL,
        PIPE_EQUAL,
        CARET_EQUAL,
        LEFT_SHIFT_EQUAL,
        RIGHT_SHIFT_EQUAL,
        WALRUS,

        LPAREN,
        RPAREN,
        LBRACKET,
        RBRACKET,
        LCBRACE,
        RCBRACE,
        COMMA,
        COLON,
        SEMICOLON,
        DOT,
        ARROW,
        ELLIPSIS,

        NEWLINE,
        INDENT,
        DEDENT,
        COMMENT,
        EOF_TOKEN,

        DEFAULT 
    };

    struct token_class {
        token_type type;
        std::string value;
        std::size_t line;
        std::size_t column;
    };
}

#endif