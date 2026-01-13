#include <cctype>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

#include "frontend/lexical.hpp"

Lexical::lexical_class::lexical_class(std::string_view source) : position(0), source(source), line(1), column(1) {
    indent.push_back(0);  

    keyword = {
        {"if", Token::token_type::KEYWORD_IF},
        {"else", Token::token_type::KEYWORD_ELSE},
        {"elif", Token::token_type::KEYWORD_ELIF},
        {"for", Token::token_type::KEYWORD_FOR},
        {"while", Token::token_type::KEYWORD_WHILE},
        {"def", Token::token_type::KEYWORD_DEF},
        {"return", Token::token_type::KEYWORD_RETURN},
        {"break", Token::token_type::KEYWORD_BREAK},
        {"continue", Token::token_type::KEYWORD_CONTINUE},
        {"pass", Token::token_type::KEYWORD_PASS},
        {"True", Token::token_type::KEYWORD_TRUE},
        {"False", Token::token_type::KEYWORD_FALSE},
        {"None", Token::token_type::KEYWORD_NONE},
        {"and", Token::token_type::KEYWORD_AND},
        {"or", Token::token_type::KEYWORD_OR},
        {"not", Token::token_type::KEYWORD_NOT},
        {"in", Token::token_type::KEYWORD_IN},
        {"is", Token::token_type::KEYWORD_IS},
        {"class", Token::token_type::KEYWORD_CLASS},
        {"import", Token::token_type::KEYWORD_IMPORT},
        {"from", Token::token_type::KEYWORD_FROM},
        {"as", Token::token_type::KEYWORD_AS},
        {"try", Token::token_type::KEYWORD_TRY},
        {"except", Token::token_type::KEYWORD_EXCEPT},
        {"finally", Token::token_type::KEYWORD_FINALLY},
        {"with", Token::token_type::KEYWORD_WITH},
        {"lambda", Token::token_type::KEYWORD_LAMBDA},
        {"yield", Token::token_type::KEYWORD_YIELD},
        {"assert", Token::token_type::KEYWORD_ASSERT},
        {"del", Token::token_type::KEYWORD_DEL},
        {"global", Token::token_type::KEYWORD_GLOBAL},
        {"nonlocal", Token::token_type::KEYWORD_NONLOCAL},
        {"raise", Token::token_type::KEYWORD_RAISE},
        {"async", Token::token_type::KEYWORD_ASYNC},
        {"await", Token::token_type::KEYWORD_AWAIT},
        {"match", Token::token_type::KEYWORD_MATCH},
        {"case", Token::token_type::KEYWORD_CASE},
    };
}

std::string Lexical::read_file(std::string_view filename) {
    std::ifstream file(filename.data());
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + std::string(filename));
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Lexical::lexical_class::handle_indentation(std::vector<Token::token_class>& tokens, std::size_t start_line) {
    std::size_t spaces = 0;
    while (position < source.length() && (source[position] == ' ' || source[position] == '\t')) {
        if (source[position] == '\t') {
            spaces += 4;  
        } else {
            spaces++;
        }
        next_token();
    }

    if (position >= source.length() || source[position] == '\n') {
        return;
    }

    std::size_t current_indent = indent.back();

    if (spaces > current_indent) {
        indent.push_back(spaces);
        tokens.push_back({Token::token_type::INDENT, "", start_line, spaces});
    } else if (spaces < current_indent) {
        while (!indent.empty() && indent.back() > spaces) {
            indent.pop_back();
            tokens.push_back({Token::token_type::DEDENT, "", start_line, spaces});
        }

        if (indent.empty() || indent.back() != spaces) {
            throw std::runtime_error("Indentation error at line " + std::to_string(line));
        }
    }
}

bool Lexical::lexical_class::is_whitespace() {
    if (position >= source.length()) {
        return false;
    }

    char c = source[position];
    return c == ' ' || c == '\t'|| c == '\r';
}

bool Lexical::lexical_class::is_string() {
    if (position >= source.length()) {
        return false;
    }

    char c = source[position];
    return c == '"' || c == '\'';
}

bool Lexical::lexical_class::is_float() {
    if (position >= source.length()) {
        return false;
    }

    std::size_t temp_pos = position;
    while (temp_pos < source.length() && std::isdigit(source[temp_pos])) {
        temp_pos++;
    }

    if (temp_pos < source.length() && source[temp_pos] == '.') {
        if (temp_pos + 1 < source.length() && std::isdigit(source[temp_pos + 1])) {
            return true; 
        }
    }

    if (source[position] == '.') {
        if (position + 1 < source.length() && std::isdigit(source[position + 1])) {
            return true; 
        }
    }

    return false;
}

bool Lexical::lexical_class::is_integer() {
    if (position >= source.length()) {
        return false;
    }

    if (!std::isdigit(source[position])) {
        return false;
    }

    return std::isdigit(source[position]); 
}

bool Lexical::lexical_class::is_identifier() {
     if (position >= source.length()) {
        return false;
    }

    char c = source[position];
    return std::isalpha(c) || c == '_';
}

void Lexical::lexical_class::next_token() {
    if (position < source.length()) {
        if (source[position] == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }
        position++;
    }
}

std::vector<Token::token_class> Lexical::lexical_class::tokenize() {
    std::vector<Token::token_class> tokens;

    bool at_line_start = true;  

    while (position < source.length()) {
        std::size_t start_line = line;
        std::size_t start_column = column;

        if (at_line_start && source[position] != '\n') {
            handle_indentation(tokens, start_line);
            at_line_start = false;
            continue;
        }

        if (is_whitespace()) {
            next_token();
            continue;
        }

        if (source[position] == '\n') {
            tokens.push_back({Token::token_type::NEWLINE, "\n", start_line, start_column});
            next_token();
            at_line_start = true;  
            continue;
        }

        if (is_float()) {
            std::size_t start = position;

            while (position < source.length() && std::isdigit(source[position])) {
                next_token();
            }
            
            if (position < source.length() && source[position] == '.') {
                next_token();
            }

            while (position < source.length() && std::isdigit(source[position])) {
                next_token();
            }

            std::string num(source.substr(start, position - start));
            tokens.push_back({Token::token_type::FLOAT_LITERAL, num, start_line, start_column});
            continue;
        }        

        if (is_integer()) {
            std::size_t start = position;
            while (position < source.length() && std::isdigit(source[position])) {
                next_token();
            }

            std::string num(source.substr(start, position - start));
            tokens.push_back({Token::token_type::INTEGER_LITERAL, num, start_line, start_column});
            continue;
        }

        if (is_string()) {
            char quote = source[position];
            next_token(); 
            std::size_t start = position;

            while (position < source.length() && source[position] != quote) {
                if (source[position] == '\\') {
                    next_token(); 
                    if (position < source.length()) {
                        next_token(); 
                    }
                } else {
                    next_token();
                }
            }

            std::string str(source.substr(start, position - start));
            if (position < source.length()) {
                next_token(); 
            }

            tokens.push_back({Token::token_type::STRING_LITERAL, str, start_line, start_column});
            continue;
        }

        if (is_identifier()) {
            std::size_t start = position;

            while (position < source.length() && (std::isalnum(source[position]) || source[position] == '_')) {
                next_token();
            }

            std::string identifier(source.substr(start, position - start));

            auto it = keyword.find(identifier);
            if (it == keyword.end()) {
                tokens.push_back({Token::token_type::IDENTIFIER, identifier, start_line, start_column});
            } else {
                tokens.push_back({it->second, identifier, start_line, start_column});
            }
            continue;
        }

        if (position + 2 < source.length()) {
            std::string three_char = std::string(source.substr(position, 3));
            if (three_char == "...") {
                tokens.push_back({Token::token_type::ELLIPSIS, three_char, start_line, start_column});
                position += 3;
                column += 3;
                continue;
            } else if (three_char == "//=") {
                tokens.push_back({Token::token_type::DOUBLE_SLASH_EQUAL, three_char, start_line, start_column});
                position += 3;
                column += 3;
                continue;
            } else if (three_char == "**=") {
                tokens.push_back({Token::token_type::POWER_EQUAL, three_char, start_line, start_column});
                position += 3;
                column += 3;
                continue;
            } else if (three_char == "<<=") {
                tokens.push_back({Token::token_type::LEFT_SHIFT_EQUAL, three_char, start_line, start_column});
                position += 3;
                column += 3;
                continue;
            } else if (three_char == ">>=") {
                tokens.push_back({Token::token_type::RIGHT_SHIFT_EQUAL, three_char, start_line, start_column});
                position += 3;
                column += 3;
                continue;
            }
        }

        if (position + 1 < source.length()) {
            std::string two_char = std::string(source.substr(position, 2));

            if (two_char == "//") {
                tokens.push_back({Token::token_type::DOUBLE_SLASH, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "**") {
                tokens.push_back({Token::token_type::POWER, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "<=") {
                tokens.push_back({Token::token_type::LESS_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == ">=") {
                tokens.push_back({Token::token_type::GREATER_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "==") {
                tokens.push_back({Token::token_type::DOUBLE_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "!=") {
                tokens.push_back({Token::token_type::NOT_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "+=") {
                tokens.push_back({Token::token_type::PLUS_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "-=") {
                tokens.push_back({Token::token_type::MINUS_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "*=") {
                tokens.push_back({Token::token_type::STAR_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "/=") {
                tokens.push_back({Token::token_type::SLASH_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "%=") {
                tokens.push_back({Token::token_type::PERCENT_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "@=") {
                tokens.push_back({Token::token_type::AT_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "&=") {
                tokens.push_back({Token::token_type::AMPERSAND_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "|=") {
                tokens.push_back({Token::token_type::PIPE_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "^=") {
                tokens.push_back({Token::token_type::CARET_EQUAL, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == ":=") {
                tokens.push_back({Token::token_type::WALRUS, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            } else if (two_char == "->") {
                tokens.push_back({Token::token_type::ARROW, two_char, start_line, start_column});
                position += 2;
                column += 2;
                continue;
            }
        }

        Token::token_type type;
        bool matched = true;

        char current = source[position];
        switch (current) {
            case '+': type = Token::token_type::PLUS; break;
            case '-': type = Token::token_type::MINUS; break;
            case '*': type = Token::token_type::STAR; break;
            case '/': type = Token::token_type::SLASH; break;
            case '%': type = Token::token_type::PERCENT; break;
            case '@': type = Token::token_type::AT; break;
            case '<': type = Token::token_type::LESS; break;
            case '>': type = Token::token_type::GREATER; break;
            case '=': type = Token::token_type::EQUAL; break;
            case '(': type = Token::token_type::LPAREN; break;
            case ')': type = Token::token_type::RPAREN; break;
            case '[': type = Token::token_type::LBRACKET; break;
            case ']': type = Token::token_type::RBRACKET; break;
            case '{': type = Token::token_type::LCBRACE; break;
            case '}': type = Token::token_type::RCBRACE; break;
            case ',': type = Token::token_type::COMMA; break;
            case ':': type = Token::token_type::COLON; break;
            case ';': type = Token::token_type::SEMICOLON; break;
            case '.': type = Token::token_type::DOT; break;
            default:
                matched = false;
                break;
        }
        
        if (matched) {
            tokens.push_back({type, std::string(1, current), start_line, start_column});
            next_token();
            continue;
        }

        tokens.push_back({Token::token_type::DEFAULT, std::string(1, current), start_line, start_column});
        next_token();
    }

    while (indent.size() > 1) {
        indent.pop_back();
        tokens.push_back({Token::token_type::DEDENT, "", line, column});
    }

    tokens.push_back({Token::token_type::EOF_TOKEN, "", line, column});

    std::cerr << "Tokenizer Completeed" << std::endl;

    return tokens;
}

std::string Lexical::lexical_class::token_type_name(Token::token_class token) {
    try {
        switch (token.type) {
            case Token::token_type::KEYWORD_FALSE: return "KEYWORD_FALSE";
            case Token::token_type::KEYWORD_NONE: return "KEYWORD_NONE";
            case Token::token_type::KEYWORD_TRUE: return "KEYWORD_TRUE";
            case Token::token_type::KEYWORD_AND: return "KEYWORD_AND";
            case Token::token_type::KEYWORD_AS: return "KEYWORD_AS";
            case Token::token_type::KEYWORD_CASE: return "KEYWORD_CASE";
            case Token::token_type::KEYWORD_MATCH: return "KEYWORD_MATCH";
            case Token::token_type::KEYWORD_ASSERT: return "KEYWORD_ASSERT";
            case Token::token_type::KEYWORD_ASYNC: return "KEYWORD_ASYNC";
            case Token::token_type::KEYWORD_AWAIT: return "KEYWORD_AWAIT";
            case Token::token_type::KEYWORD_BREAK: return "KEYWORD_BREAK";
            case Token::token_type::KEYWORD_CLASS: return "KEYWORD_CLASS";
            case Token::token_type::KEYWORD_CONTINUE: return "KEYWORD_CONTINUE";
            case Token::token_type::KEYWORD_DEF: return "KEYWORD_DEF";
            case Token::token_type::KEYWORD_DEL: return "KEYWORD_DEL";
            case Token::token_type::KEYWORD_ELIF: return "KEYWORD_ELIF";
            case Token::token_type::KEYWORD_ELSE: return "KEYWORD_ELSE";
            case Token::token_type::KEYWORD_EXCEPT: return "KEYWORD_EXCEPT";
            case Token::token_type::KEYWORD_FINALLY: return "KEYWORD_FINALLY";
            case Token::token_type::KEYWORD_FOR: return "KEYWORD_FOR";
            case Token::token_type::KEYWORD_FROM: return "KEYWORD_FROM";
            case Token::token_type::KEYWORD_GLOBAL: return "KEYWORD_GLOBAL";
            case Token::token_type::KEYWORD_IF: return "KEYWORD_IF";
            case Token::token_type::KEYWORD_IMPORT: return "KEYWORD_IMPORT";
            case Token::token_type::KEYWORD_IN: return "KEYWORD_IN";
            case Token::token_type::KEYWORD_IS: return "KEYWORD_IS";
            case Token::token_type::KEYWORD_LAMBDA: return "KEYWORD_LAMBDA";
            case Token::token_type::KEYWORD_NONLOCAL: return "KEYWORD_NONLOCAL";
            case Token::token_type::KEYWORD_NOT: return "KEYWORD_NOT";
            case Token::token_type::KEYWORD_OR: return "KEYWORD_OR";
            case Token::token_type::KEYWORD_PASS: return "KEYWORD_PASS";
            case Token::token_type::KEYWORD_RAISE: return "KEYWORD_RAISE";
            case Token::token_type::KEYWORD_RETURN: return "KEYWORD_RETURN";
            case Token::token_type::KEYWORD_TRY: return "KEYWORD_TRY";
            case Token::token_type::KEYWORD_WHILE: return "KEYWORD_WHILE";
            case Token::token_type::KEYWORD_WITH: return "KEYWORD_WITH";
            case Token::token_type::KEYWORD_YIELD: return "KEYWORD_YIELD";
            case Token::token_type::IDENTIFIER: return "IDENTIFIER";
            case Token::token_type::INTEGER_LITERAL: return "INTEGER_LITERAL";
            case Token::token_type::FLOAT_LITERAL: return "FLOAT_LITERAL";
            case Token::token_type::STRING_LITERAL: return "STRING_LITERAL";
            case Token::token_type::BYTES_LITERAL: return "BYTES_LITERAL";
            case Token::token_type::PLUS: return "PLUS";
            case Token::token_type::MINUS: return "MINUS";
            case Token::token_type::STAR: return "STAR";
            case Token::token_type::SLASH: return "SLASH";
            case Token::token_type::DOUBLE_SLASH: return "DOUBLE_SLASH";
            case Token::token_type::PERCENT: return "PERCENT";
            case Token::token_type::POWER: return "POWER";
            case Token::token_type::AT: return "AT";
            case Token::token_type::LESS: return "LESS";
            case Token::token_type::GREATER: return "GREATER";
            case Token::token_type::LESS_EQUAL: return "LESS_EQUAL";
            case Token::token_type::GREATER_EQUAL: return "GREATER_EQUAL";
            case Token::token_type::DOUBLE_EQUAL: return "DOUBLE_EQUAL";
            case Token::token_type::NOT_EQUAL: return "NOT_EQUAL";
            case Token::token_type::EQUAL: return "EQUAL";
            case Token::token_type::PLUS_EQUAL: return "PLUS_EQUAL";
            case Token::token_type::MINUS_EQUAL: return "MINUS_EQUAL";
            case Token::token_type::STAR_EQUAL: return "STAR_EQUAL";
            case Token::token_type::SLASH_EQUAL: return "SLASH_EQUAL";
            case Token::token_type::DOUBLE_SLASH_EQUAL: return "DOUBLE_SLASH_EQUAL";
            case Token::token_type::PERCENT_EQUAL: return "PERCENT_EQUAL";
            case Token::token_type::POWER_EQUAL: return "POWER_EQUAL";
            case Token::token_type::AT_EQUAL: return "AT_EQUAL";
            case Token::token_type::AMPERSAND_EQUAL: return "AMPERSAND_EQUAL";
            case Token::token_type::PIPE_EQUAL: return "PIPE_EQUAL";
            case Token::token_type::CARET_EQUAL: return "CARET_EQUAL";
            case Token::token_type::LEFT_SHIFT_EQUAL: return "LEFT_SHIFT_EQUAL";
            case Token::token_type::RIGHT_SHIFT_EQUAL: return "RIGHT_SHIFT_EQUAL";
            case Token::token_type::WALRUS: return "WALRUS";
            case Token::token_type::LPAREN: return "LPAREN";
            case Token::token_type::RPAREN: return "RPAREN";
            case Token::token_type::LBRACKET: return "LBRACKET";
            case Token::token_type::RBRACKET: return "RBRACKET";
            case Token::token_type::LCBRACE: return "LCBRACE";
            case Token::token_type::RCBRACE: return "RCBRACE";
            case Token::token_type::COMMA: return "COMMA";
            case Token::token_type::COLON: return "COLON";
            case Token::token_type::SEMICOLON: return "SEMICOLON";
            case Token::token_type::DOT: return "DOT";
            case Token::token_type::ARROW: return "ARROW";
            case Token::token_type::ELLIPSIS: return "ELLIPSIS";
            case Token::token_type::NEWLINE: return "NEWLINE";
            case Token::token_type::INDENT: return "INDENT";
            case Token::token_type::DEDENT: return "DEDENT";
            case Token::token_type::COMMENT: return "COMMENT";
            case Token::token_type::EOF_TOKEN: return "EOF_TOKEN";
            case Token::token_type::DEFAULT: return "DEFAULT";
            default: return "UNKNOWN";
        }
    } catch (const std::exception& e) {
        return "ERROR";
    }
}
