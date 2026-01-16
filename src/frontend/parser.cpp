#include <iostream>
#include <stdexcept>
#include <format>

#include "frontend/parser.hpp"

/*
Recursive descent order (higher precedence = parsed deeper)

parse_program()        - Top level
parse_statement()      - Statement dispatcher
parse_assignment()     - Assignments
parse_term()           - Addition / subtraction (lowest precedence)
parse_factor()         - Multiplication / division
parse_power()          - Exponentiation (right-associative)
parse_expression()     - Literals, identifiers, primaries
parse_misc_expression()- Attribute access, function calls
parse_function_def()   - Function definitions
parse_return_stmt()    - Return statements
parse_if_stmt()        - If statements
*/

Parser::parser_class::parser_class(Lexical::lexical_class& lexer) : current_pos(0)  {
    tokens = lexer.tokenize();

    std::cerr << "Parser constructor called" << std::endl;
}

bool Parser::parser_class::consume(const Token::token_type& type) {
    if (match(type)) {
        current_pos++;
        return true;
    }

    return false;
}

Token::token_class& Parser::parser_class::previous_token() {
    return tokens[--current_pos];
}

Token::token_class& Parser::parser_class::current_token() {
    return tokens[current_pos];
}

Ast::ast_class& Parser::parser_class::get_ast() {
    return ast_tree;
}

bool Parser::parser_class::match(const Token::token_type& type) {
    return type == current_token().type;
}

bool Parser::parser_class::is_at_end() {
    return current_pos >= tokens.size() || current_token().type == Token::token_type::EOF_TOKEN;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse() {
    auto root = std::make_unique<Ast::ast_node>(Ast::node_type::PROGRAM);

   //try {
        while (!is_at_end()) {
            auto stmt = parse_statement();
            if (stmt) {
                root->add_child(std::move(stmt));
            }
        }
    //} catch (const std::exception& e) {
    //   std::cerr << "Parse error: " << e.what() << std::endl;
    // throw;
    //}

    ast_tree.set_root(std::move(root));

    return nullptr;
}

void Parser::parser_class::consume_newline() {
    consume(Token::token_type::COLON);
    consume(Token::token_type::NEWLINE);
    if (!match(Token::token_type::INDENT)) {
        throw std::runtime_error("Expected indentation after ':' at line " + std::to_string(current_token().line));
    }
    consume(Token::token_type::INDENT);
}


void Parser::parser_class::consume_line() {
   consume(Token::token_type::NEWLINE);
    if (!match(Token::token_type::INDENT)) {
        throw std::runtime_error("Expected indentation at line " + std::to_string(current_token().line));
    }
   consume(Token::token_type::INDENT);
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_expression() {
    std::unique_ptr<Ast::ast_node> node;

    switch (current_token().type) {
        case Token::token_type::INTEGER_LITERAL: {
            node = std::make_unique<Ast::ast_node>(
                Ast::node_type::INTEGER_LITERAL,
                Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
            );
            consume(Token::token_type::INTEGER_LITERAL);
            break;
        }

        case Token::token_type::FLOAT_LITERAL: {
            node = std::make_unique<Ast::ast_node>(
                Ast::node_type::FLOAT_LITERAL,
                Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
            );
            consume(Token::token_type::FLOAT_LITERAL);
            break;
        }

        case Token::token_type::STRING_LITERAL: {
            node = std::make_unique<Ast::ast_node>(
                Ast::node_type::STRING_LITERAL,
                Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
            );
            consume(Token::token_type::STRING_LITERAL);
            break;
        }

        case Token::token_type::IDENTIFIER: {
            node = std::make_unique<Ast::ast_node>(
                Ast::node_type::IDENTIFIER,
                Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
            );
            consume(Token::token_type::IDENTIFIER);
            break;
        }

        case Token::token_type::KEYWORD_SELF: {
            node = std::make_unique<Ast::ast_node>(
                Ast::node_type::SELF,
                Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
            );
            consume(Token::token_type::KEYWORD_SELF);
            break;
        }

        case Token::token_type::LBRACKET: {
            node = parse_list();
            break;
        }

        default:
            throw std::runtime_error("Unexpected token: " + current_token().value +
                                   " at line " + std::to_string(current_token().line));
    }

    return parse_misc_expression(std::move(node));
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_misc_expression(std::unique_ptr<Ast::ast_node> node) {
    while (match(Token::token_type::DOT) || match(Token::token_type::LPAREN)) {
        if (match(Token::token_type::DOT)) {
            consume(Token::token_type::DOT);

            if (!match(Token::token_type::IDENTIFIER)) {
                throw std::runtime_error("Expected identifier after '.' at line " +
                                       std::to_string(current_token().line));
            }

            auto attr_node = std::make_unique<Ast::ast_node>(
                Ast::node_type::ATTRIBUTE_EXPR,
                Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
            );
            consume(Token::token_type::IDENTIFIER);

            attr_node->add_child(std::move(node));
            node = std::move(attr_node);
        } else if (match(Token::token_type::LPAREN)) {
            auto call_node = std::make_unique<Ast::ast_node>(
                Ast::node_type::CALL_EXPR,
                Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
            );
            consume(Token::token_type::LPAREN);

            call_node->add_child(std::move(node));

            auto arg_list = std::make_unique<Ast::ast_node>(
                Ast::node_type::ARGUMENT_LIST,
                Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
            );

            if (!match(Token::token_type::RPAREN)) {
                arg_list->add_child(parse_term());

                while (match(Token::token_type::COMMA)) {
                    consume(Token::token_type::COMMA);

                    if (match(Token::token_type::RPAREN)) {
                        break;
                    }

                    arg_list->add_child(parse_term());
                }
            }

            consume(Token::token_type::RPAREN);
            call_node->add_child(std::move(arg_list));
            node = std::move(call_node);
        }
    }

    return node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_term() {
    auto left = parse_factor();

    while (match(Token::token_type::PLUS) || match(Token::token_type::MINUS)) {
        auto op_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::BINARY_OP,
            Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
        );
        current_pos++;

        op_node->add_child(std::move(left));
        op_node->add_child(parse_factor());

        left = std::move(op_node);
    }

    return parse_misc_expression(std::move(left));
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_equality() {
    auto left = parse_term();

    while (match(Token::token_type::DOUBLE_EQUAL) || match(Token::token_type::NOT_EQUAL)) {
         auto op_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::EQUALITY_OP,
            Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
        );
        current_pos++;

        op_node->add_child(std::move(left));
        op_node->add_child(parse_term());

        left = std::move(op_node);
    }

    return left;
} 

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_factor() {
    auto left = parse_power();

    while (match(Token::token_type::STAR) || match(Token::token_type::SLASH) ||
           match(Token::token_type::DOUBLE_SLASH) || match(Token::token_type::PERCENT)) {
        auto op_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::BINARY_OP,
            Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
        );
        current_pos++;

        op_node->add_child(std::move(left));
        op_node->add_child(parse_power());

        left = std::move(op_node);
    }

    return left;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_power() {
    auto left = parse_expression();

    if (match(Token::token_type::POWER)) {
        auto op_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::BINARY_OP,
            Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
        );
        current_pos++;

        op_node->add_child(std::move(left));
        op_node->add_child(parse_power());  

        return op_node;
    }

    return left;
}

/* std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_bitwise() {
    
} */

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_statement() {
    switch (current_token().type) {
        case Token::token_type::NEWLINE:
            consume(Token::token_type::NEWLINE);
            return nullptr;

        case Token::token_type::KEYWORD_DEF:
            return parse_function_def();

        case Token::token_type::KEYWORD_CLASS:
            return parse_class();

        case Token::token_type::KEYWORD_IF:
            return parse_if_stmt();

        case Token::token_type::KEYWORD_WHILE:
            return parse_while_stmt();

        case Token::token_type::KEYWORD_FOR:
            return parse_for_stmt();

        case Token::token_type::KEYWORD_MATCH:
            return parse_match_stmt();

        case Token::token_type::KEYWORD_CASE:
            return parse_case();

        case Token::token_type::KEYWORD_RETURN:
            return parse_return_stmt();
        
        case Token::token_type::KEYWORD_PASS: 
            return parse_pass();
        
        case Token::token_type::KEYWORD_TRY:
            return parse_try();

        case Token::token_type::KEYWORD_BREAK:
            return parse_break();

        case Token::token_type::KEYWORD_CONTINUE:
            return parse_continue();

        default:
            return parse_assignment();
    }
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_try() {
    consume(Token::token_type::KEYWORD_TRY);

    auto try_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::TRY_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    consume_newline();
    auto try_body = std::make_unique<Ast::ast_node>(
        Ast::node_type::BLOCK,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    while (!match(Token::token_type::DEDENT) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            try_body->add_child(std::move(stmt));
        }
    }

    consume(Token::token_type::DEDENT);
    try_node->add_child(std::move(try_body));

    if (match(Token::token_type::KEYWORD_EXCEPT)) {
        consume(Token::token_type::KEYWORD_EXCEPT);

        auto expect_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::EXCEPT_STMT,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );

        consume_newline();
        auto expect_body = std::make_unique<Ast::ast_node>(
            Ast::node_type::BLOCK,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );

        while (!match(Token::token_type::DEDENT) && !is_at_end()) {
            auto stmt = parse_statement();
            if (stmt) {
                expect_body->add_child(std::move(stmt));
            }
        }

        consume(Token::token_type::DEDENT);
        expect_node->add_child(std::move(expect_body)); 

        try_node->add_child(std::move(expect_node));
    }

    if (match(Token::token_type::KEYWORD_FINALLY)) {
         consume(Token::token_type::KEYWORD_FINALLY);

        auto finally_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::FINALLY_STMT,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );

        consume_newline();
        auto finally_body = std::make_unique<Ast::ast_node>(
            Ast::node_type::BLOCK,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );

        while (!match(Token::token_type::DEDENT) && !is_at_end()) {
            auto stmt = parse_statement();
            if (stmt) {
                finally_body->add_child(std::move(stmt));
            }
        }

        consume(Token::token_type::DEDENT);
        finally_node->add_child(std::move(finally_body));

        try_node->add_child(std::move(finally_node));
    }

    if (match(Token::token_type::KEYWORD_ELSE)) {
        auto else_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::ELSE_STMT,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );
        consume(Token::token_type::KEYWORD_ELSE);

        consume_newline();
        auto else_body = std::make_unique<Ast::ast_node>(
            Ast::node_type::BLOCK,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );

        while (!match(Token::token_type::DEDENT) && !is_at_end()) {
            auto stmt = parse_statement();
            if (stmt) {
                else_body->add_child(std::move(stmt));
            }
        }

        consume(Token::token_type::DEDENT);
        else_node->add_child(std::move(else_body));

        try_node->add_child(std::move(else_node));
    }

    return try_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_pass() {
    auto pass_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::PASS_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );
    consume(Token::token_type::KEYWORD_PASS);
    return pass_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_assignment() {
    auto left = parse_equality();

    if (match(Token::token_type::EQUAL)) {
        auto assign_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::ASSIGNMENT,
            Token::token_class{Token::token_type::DEFAULT, "=", current_token().line, current_token().column}
        );
        consume(Token::token_type::EQUAL);

        assign_node->add_child(std::move(left));
        assign_node->add_child(parse_assignment());

        return assign_node;
    }

    if (match(Token::token_type::PLUS_EQUAL) ||
        match(Token::token_type::MINUS_EQUAL) ||
        match(Token::token_type::STAR_EQUAL) ||
        match(Token::token_type::SLASH_EQUAL)) {
        auto aug_assign_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::AUGMENTED_ASSIGNMENT,
            Token::token_class{current_token().type, current_token().value, current_token().line, current_token().column}
        );
        consume(current_token().type);

        aug_assign_node->add_child(std::move(left));
        aug_assign_node->add_child(parse_assignment());  

        return aug_assign_node;
    }

    return left;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_return_stmt() {
    auto ret_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::RETURN_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );
    consume(Token::token_type::KEYWORD_RETURN);

    if (!is_at_end() && !match(Token::token_type::NEWLINE)) {
        ret_node->add_child(parse_expression());
    }

    return ret_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_function_def() {
    consume(Token::token_type::KEYWORD_DEF);

    if (!match(Token::token_type::IDENTIFIER)) {
        throw std::runtime_error("Expected function name at line " +
                               std::to_string(current_token().line));
    }

    auto func_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::FUNCTION_DEF,
        Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
    );

    if (match(Token::token_type::KEYWORD_INIT)) {
        consume(Token::token_type::KEYWORD_INIT);
    } else {
        consume(Token::token_type::IDENTIFIER);
    }

    consume(Token::token_type::LPAREN);       

    auto param_list = std::make_unique<Ast::ast_node>(
        Ast::node_type::PARAMETER_LIST,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    if (!match(Token::token_type::RPAREN)) {
        if (match(Token::token_type::IDENTIFIER) || match(Token::token_type::KEYWORD_SELF)) {
            auto param = std::make_unique<Ast::ast_node>(
                Ast::node_type::PARAMETER,
                Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
            );
            if (match(Token::token_type::KEYWORD_SELF)) {
                consume(Token::token_type::KEYWORD_SELF);
            } else {
                consume(Token::token_type::IDENTIFIER);
            }
            param_list->add_child(std::move(param));
        }

        while (match(Token::token_type::COMMA)) {
            consume(Token::token_type::COMMA);
            if (match(Token::token_type::IDENTIFIER) || match(Token::token_type::KEYWORD_SELF)) {
                auto param = std::make_unique<Ast::ast_node>(
                    Ast::node_type::PARAMETER,
                    Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
                );
                if (match(Token::token_type::KEYWORD_SELF)) {
                    consume(Token::token_type::KEYWORD_SELF);
                } else {
                    consume(Token::token_type::IDENTIFIER);
                }
                param_list->add_child(std::move(param));
            }
        }
    }

    consume(Token::token_type::RPAREN);
    consume(Token::token_type::COLON);

    func_node->add_child(std::move(param_list));

    auto body = std::make_unique<Ast::ast_node>(
        Ast::node_type::BLOCK,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );
    
    consume_line();
    while (!match(Token::token_type::DEDENT) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            body->add_child(std::move(stmt));
        }
    }

    consume(Token::token_type::DEDENT);
    func_node->add_child(std::move(body));

    return func_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_class() {
    consume(Token::token_type::KEYWORD_CLASS);
    auto class_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::CLASS_DEF,
        Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
    );
    consume(Token::token_type::IDENTIFIER);

    consume_newline();

    auto body = std::make_unique<Ast::ast_node>(
        Ast::node_type::BLOCK,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    while (!match(Token::token_type::DEDENT) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            body->add_child(std::move(stmt));
        }
    }

    consume(Token::token_type::DEDENT);
    class_node->add_child(std::move(body));

    return class_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_break() {
    auto break_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::BREAK_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );    
    consume(Token::token_type::KEYWORD_BREAK);
    return break_node; 
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_continue() {
    auto continue_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::CONTINUE_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );    
    consume(Token::token_type::KEYWORD_CONTINUE);
    return continue_node; 
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_if_stmt() {
    auto if_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::IF_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );
    consume(Token::token_type::KEYWORD_IF);

    if_node->add_child(parse_equality());

    consume_newline();
    auto if_body = std::make_unique<Ast::ast_node>(
        Ast::node_type::BLOCK,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    while (!match(Token::token_type::DEDENT) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            if_body->add_child(std::move(stmt));
        }
    }

    consume(Token::token_type::DEDENT);
    if_node->add_child(std::move(if_body));

    while (match(Token::token_type::KEYWORD_ELIF)) {
        auto elif_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::ELIF_STMT,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );
        consume(Token::token_type::KEYWORD_ELIF);

        elif_node->add_child(parse_expression());

        consume_newline();
        auto elif_body = std::make_unique<Ast::ast_node>(
            Ast::node_type::BLOCK,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );

        while (!match(Token::token_type::DEDENT) && !is_at_end()) {
            auto stmt = parse_statement();
            if (stmt) {
                elif_body->add_child(std::move(stmt));
            }
        }

        consume(Token::token_type::DEDENT);
        elif_node->add_child(std::move(elif_body));

        if_node->add_child(std::move(elif_node));
    }

    if (match(Token::token_type::KEYWORD_ELSE)) {
        auto else_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::ELSE_STMT,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );
        consume(Token::token_type::KEYWORD_ELSE);

        consume_newline();
        auto else_body = std::make_unique<Ast::ast_node>(
            Ast::node_type::BLOCK,
            Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
        );

        while (!match(Token::token_type::DEDENT) && !is_at_end()) {
            auto stmt = parse_statement();
            if (stmt) {
                else_body->add_child(std::move(stmt));
            }
        }

        consume(Token::token_type::DEDENT);
        else_node->add_child(std::move(else_body));

        if_node->add_child(std::move(else_node));
    }

    return if_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_while_stmt() {
    auto while_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::WHILE_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );
    consume(Token::token_type::KEYWORD_WHILE);

    while_node->add_child(parse_expression());

    consume_newline();
    auto body = std::make_unique<Ast::ast_node>(
        Ast::node_type::BLOCK,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    while (!match(Token::token_type::DEDENT) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            body->add_child(std::move(stmt));
        }
    }

    consume(Token::token_type::DEDENT);
    while_node->add_child(std::move(body));

    return while_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_for_stmt() {
    auto for_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::FOR_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );
    consume(Token::token_type::KEYWORD_FOR);

    if (match(Token::token_type::IDENTIFIER)) {
        auto var_node = std::make_unique<Ast::ast_node>(
            Ast::node_type::IDENTIFIER,
            Token::token_class{Token::token_type::DEFAULT, current_token().value, current_token().line, current_token().column}
        );
        consume(Token::token_type::IDENTIFIER);
        for_node->add_child(std::move(var_node));
    }

    consume(Token::token_type::KEYWORD_IN);

    for_node->add_child(parse_expression());

    consume_newline();
    auto body = std::make_unique<Ast::ast_node>(
        Ast::node_type::BLOCK,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    while (!match(Token::token_type::DEDENT) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            body->add_child(std::move(stmt));
        }
    }

    consume(Token::token_type::DEDENT);
    for_node->add_child(std::move(body));

    return for_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_match_stmt() {
    auto match_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::MATCH_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );
    consume(Token::token_type::KEYWORD_MATCH);

    match_node->add_child(parse_expression());

    consume_newline();
    while (match(Token::token_type::KEYWORD_CASE) && !is_at_end()) {
        auto case_stmt = parse_case();
        if (case_stmt) {
            match_node->add_child(std::move(case_stmt));
        }
    }

    consume(Token::token_type::DEDENT);

    return match_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_case() {
    auto case_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::CASE_STMT,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );
    consume(Token::token_type::KEYWORD_CASE);

    case_node->add_child(parse_expression());

    consume_newline();
    auto case_body = std::make_unique<Ast::ast_node>(
        Ast::node_type::BLOCK,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    while (!match(Token::token_type::DEDENT) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            case_body->add_child(std::move(stmt));
        }
    }

    consume(Token::token_type::DEDENT);
    case_node->add_child(std::move(case_body));

    return case_node;
}

std::unique_ptr<Ast::ast_node> Parser::parser_class::parse_list() {
    auto list_node = std::make_unique<Ast::ast_node>(
        Ast::node_type::LIST,
        Token::token_class{Token::token_type::DEFAULT, "", current_token().line, current_token().column}
    );

    consume(Token::token_type::LBRACKET);

    if (match(Token::token_type::RBRACKET)) {
        consume(Token::token_type::RBRACKET);
        return list_node;
    }

    list_node->add_child(parse_expression());

    while (match(Token::token_type::COMMA)) {
        consume(Token::token_type::COMMA);

        if (match(Token::token_type::RBRACKET)) {
            break;
        }

        list_node->add_child(parse_expression());
    }

    consume(Token::token_type::RBRACKET);
    return list_node;
}
