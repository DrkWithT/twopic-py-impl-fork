#ifndef TOKEN_HPP
#define TOKEN_HPP 

#include <string>
#include <unordered_map>

/* The token determines what type a certain phrase or word is going to be for a character*/

namespace Token {
    enum class token_type {
        KEYWORD_FALSE, KEYWORD_NONE, KEYWORD_TRUE,
        KEYWORD_AND, KEYWORD_AS, KEYWORD_ASSERT, KEYWORD_ASYNC, KEYWORD_AWAIT,
        KEYWORD_BREAK,
        KEYWORD_CLASS, KEYWORD_CONTINUE,
        KEYWORD_DEF, KEYWORD_DEL,
        KEYWORD_ELIF, KEYWORD_ELSE, KEYWORD_EXCEPT,
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

        VARIABLE,
        INTEGER_LITERAL,
        FLOAT_LITERAL,
        STRING_LITERAL,
        BYTES_LITERAL,

        PLUS,           // +
        MINUS,          // -
        STAR,           // *
        SLASH,          // /
        DOUBLE_SLASH,   // //
        PERCENT,        // %
        POWER,          // **
        AT,             // @

        LESS,           // <
        GREATER,        // >
        LESS_EQUAL,     // <=
        GREATER_EQUAL,  // >=
        DOUBLE_EQUAL,   // ==
        NOT_EQUAL,      // !=

        EQUAL,          // =
        PLUS_EQUAL,     // +=
        MINUS_EQUAL,    // -=
        STAR_EQUAL,     // *=
        SLASH_EQUAL,    // /=
        DOUBLE_SLASH_EQUAL,  // //=
        PERCENT_EQUAL,  // %=
        POWER_EQUAL,    // **=
        AT_EQUAL,       // @=
        AMPERSAND_EQUAL,     // &=
        PIPE_EQUAL,     // |=
        CARET_EQUAL,    // ^=
        LEFT_SHIFT_EQUAL,    // <<=
        RIGHT_SHIFT_EQUAL,   // >>=
        WALRUS,         // :=

        LPAREN,         // (
        RPAREN,         // )
        LBRACKET,       // [
        RBRACKET,       // ]
        LBRACE,         // {
        RBRACE,         // }
        COMMA,          // ,
        COLON,          // :
        SEMICOLON,      // ;
        DOT,            // .
        ARROW,          // ->
        ELLIPSIS,       // ...

        // Special Tokens
        NEWLINE,
        INDENT,
        DEDENT,
        COMMENT,
        EOF_TOKEN,

        ERROR
    };

    struct token_class {
        token_type type;
        std::string value;
        size_t line;
        size_t column;
    };
}

#endif