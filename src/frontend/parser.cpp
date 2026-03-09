#include <stdexcept>
#include <format>

#include "frontend/parser.hpp"

namespace TwoPy::Frontend {

/*
Recursive descent order

parse_program()        - Top level
parse_statement()      - Statement dispatcher
parse_assignment()     - Assignments
parse_logical_or()      // or
parse_logical_and()     // and
parse_equality()       - Equality (==, !=)
parse_comparator()     - Comparison (<, >, <=, >=)
parse_bitwise()        - Bitwise ops (|, ^, &, <<, >>)
parse_term()           - Addition / subtraction
parse_factor()         - Multiplication / division
parse_power()          - Exponentiation
parse_expression()     - Literals, identifier
*/

parser_class::parser_class(lexical_class& lexer) : current_pos(0),  m_previous_pos(0) {
    tokens = lexer.tokenize();
}

token_class& parser_class::current_token() {
    return tokens[current_pos];
}

token_class& parser_class::previous_token() {
    return tokens[m_previous_pos];
}

bool parser_class::match(const token_type& type) {
    return type == current_token().type;
}

bool parser_class::is_at_end() {
    return current_pos >= tokens.size() || current_token().type == token_type::EOF_TOKEN;
}

Program parser_class::parse() {
    Program program;

    try {
        while (!is_at_end()) {
            auto stmt = parse_statement();
            if (stmt) {
                program.statements.push_back(std::move(stmt));
            }
        }
    } catch (const std::exception& e) {
        debug_syntax_error();
    }
    
    return program;
}

Block parser_class::parse_block() {
    Block block;
    block.token = current_token();

    while (!match(token_type::DEDENT) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            block.statements.push_back(std::move(stmt));
        }
    }

    consume(token_type::DEDENT);
    return block;
}

ExprPtr parser_class::parse_expression_types() {
    ExprPtr expr {};

    switch (current_token().type) {
        case token_type::INTEGER_LITERAL: {
            IntegerLiteral lit{current_token()};
            expr = std::make_unique<ExprNode>(ExprNode{lit});
            consume(token_type::INTEGER_LITERAL);
            break;
        }

        case token_type::FLOAT_LITERAL: {
            FloatLiteral lit{current_token()};
            expr = std::make_unique<ExprNode>(ExprNode{lit});
            consume(token_type::FLOAT_LITERAL);
            break;
        }

        case token_type::STRING_LITERAL: {
            StringLiteral lit{current_token()};
            expr = std::make_unique<ExprNode>(ExprNode{lit});
            consume(token_type::STRING_LITERAL);
            break;
        }

        case token_type::KEYWORD_TRUE: {
            BoolLiteral lit{current_token()};
            expr = std::make_unique<ExprNode>(ExprNode{lit});
            consume(token_type::KEYWORD_TRUE);
            break;
        }

        case token_type::KEYWORD_FALSE: {
            BoolLiteral lit{current_token()};
            expr = std::make_unique<ExprNode>(ExprNode{lit});
            consume(token_type::KEYWORD_FALSE);
            break;
        }

        case token_type::IDENTIFIER: {
            Identifier id{current_token()};
            auto id_expr = std::make_unique<ExprNode>(ExprNode{id});
            consume(token_type::IDENTIFIER);

            if (match(token_type::LPAREN)) {
                if (valid_constructor) {
                    expr = parse_constructor_call(std::move(id_expr));
                    valid_constructor = false;
                } else {
                    expr = parse_call_expr(std::move(id_expr));
                }
            } else {
                expr = std::move(id_expr);
            }

            break;
        }

        case token_type::LBRACKET:
            expr = parse_list();
            break;

        case token_type::LCBRACE:
            expr = parse_dict();
            break;

        case token_type::LPAREN: {
            consume(token_type::LPAREN);
            expr = parse_logical_or();
            consume(token_type::RPAREN);
            break;
        }

        case token_type::KEYWORD_SELF:
            expr = parse_self();
            break;

        default:
            debug_syntax_error();
    }

    return expr;
}

ExprPtr parser_class::parse_attribute_expr() {
    Identifier inst_var{previous_token()};
    
    consume(token_type::DOT);

    if (!match(token_type::IDENTIFIER)) {
        debug_syntax_error();
    }


    Identifier attr{current_token()};
    consume();

    AttributeExpr attr_expr{current_token(), inst_var, attr};
    auto attr_node = std::make_unique<ExprNode>(ExprNode{std::move(attr_expr)});
    return attr_node;
}

ExprPtr parser_class::parse_call_expr(ExprPtr callee) {
    token_class token = current_token();
    consume(token_type::LPAREN);

    CallExpr call{token, std::move(callee), {}};

    if (!match(token_type::RPAREN)) {
        call.arguments.push_back(parse_term());

        while (match(token_type::COMMA)) {
            consume(token_type::COMMA);

            if (match(token_type::RPAREN)) {
                break;
            }

            call.arguments.push_back(parse_term());
        }
    }

    consume();
    auto call_node = std::make_unique<ExprNode>(ExprNode{std::move(call)});
    return call_node;
}

ExprPtr parser_class::parse_constructor_call(ExprPtr constructor) {
    token_class token = current_token();
    consume(token_type::LPAREN);

    ConstructorCallExpr con{token, std::move(constructor), {}};

    if (!match(token_type::RPAREN)) {
        con.arguments.push_back(parse_term());

        while (match(token_type::COMMA)) {
            consume(token_type::COMMA);

            if (match(token_type::RPAREN)) {
                break;
            }

            con.arguments.push_back(parse_term());
        }
    }

    consume();
    auto con_node = std::make_unique<ExprNode>(ExprNode{std::move(con)});
    return con_node;
}

ExprPtr parser_class::parse_comparator() {
    auto left = parse_bitwise();

    if (match(token_type::GREATER, token_type::GREATER_EQUAL, token_type::LESS, token_type::LESS_EQUAL)) {
        token_class op { current_token() };
        consume(current_token().type);

        ComparisonOp comp{op, std::move(left), parse_comparator()};
        auto comp_node = std::make_unique<ExprNode>(ExprNode{std::move(comp)});
        return comp_node;
    }

    return left;
}

ExprPtr parser_class::parse_term() {
    auto left = parse_factor();

    while (match(token_type::PLUS, token_type::MINUS)) {
        token_class op { current_token() };
        consume(current_token().type);

        TermOp term{op, std::move(left), parse_factor()};
        left = std::make_unique<ExprNode>(ExprNode{std::move(term)});
    }

    return left;
}

ExprPtr parser_class::parse_equality() {
    auto left = parse_comparator();

    while (match(token_type::DOUBLE_EQUAL, token_type::NOT_EQUAL)) {
        token_class op { current_token() };
        consume();

        EqualityOp eq{op, std::move(left), parse_comparator()};
        left = std::make_unique<ExprNode>(ExprNode{std::move(eq)});
    }

    return left;
}

ExprPtr parser_class::parse_factor() {
    auto left = parse_power();

    while (match(token_type::STAR, token_type::SLASH, token_type::DOUBLE_SLASH, token_type::PERCENT)) {
        token_class op { current_token() };
        consume();

        FactorOp factor{op, std::move(left), parse_power()};
        left = std::make_unique<ExprNode>(ExprNode{std::move(factor)});
    }

    return left;
}

ExprPtr parser_class::parse_power() {
    auto base = parse_expression_types();

    if (match(token_type::POWER)) {
        token_class op { current_token() };
        consume();

        PowerOp power{op, std::move(base), parse_power()};
        auto power_node = std::make_unique<ExprNode>(ExprNode{std::move(power)});
        return power_node;
    }

    return base;
}

ExprPtr parser_class::parse_bitwise() {
    auto left = parse_term();

    while (match(token_type::PIPE, token_type::CARET, token_type::AMPERSAND, token_type::LEFT_SHIFT, token_type::RIGHT_SHIFT)) {
        token_class op { current_token() };
        consume();

        BitwiseOp bitwise{op, std::move(left), parse_term()};
        left = std::make_unique<ExprNode>(ExprNode{std::move(bitwise)});
    }

    return left;
}

StmtPtr parser_class::parse_lambda() {
    token_class token {current_token()};
    consume(token_type::KEYWORD_LAMBDA);

    ParameterList params;

    if (!match(token_type::COLON)) {
        if (match(token_type::IDENTIFIER)) {
            params.params.push_back(Parameter{current_token()});
            consume(token_type::IDENTIFIER);
        }

        while (match(token_type::COMMA)) {
            consume(token_type::COMMA);
            if (match(token_type::IDENTIFIER)) {
                params.params.push_back(Parameter{current_token()});
                consume(token_type::IDENTIFIER);
            }
        }
    }
    consume(token_type::COLON);

    std::vector<StmtPtr> body;
    while (!match(token_type::NEWLINE) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            body.push_back(std::move(stmt));
        }
    }

    consume(token_type::NEWLINE);

    LambdaStmt lambda{token, std::move(params), std::move(body)};
    auto lambda_node = std::make_unique<StmtNode>(StmtNode{std::move(lambda)});
    return lambda_node;
}

StmtPtr parser_class::parse_statement() {
    switch (current_token().type) {
        case token_type::NEWLINE:
            consume(token_type::NEWLINE);
            return nullptr;

        case token_type::KEYWORD_DEF:
            return parse_function_def();

        case token_type::KEYWORD_CLASS:
            return parse_class();

        case token_type::KEYWORD_IF:
            return parse_if_stmt();

        case token_type::KEYWORD_WHILE:
            return parse_while_stmt();

        case token_type::KEYWORD_FOR:
            return parse_for_stmt();

        case token_type::KEYWORD_MATCH:
            return parse_match_stmt();

        case token_type::KEYWORD_CASE:
            return parse_case();

        case token_type::KEYWORD_RETURN:
            return parse_return_stmt();

        case token_type::KEYWORD_PASS:
            return parse_pass();

        case token_type::KEYWORD_TRY:
            return parse_try();

        case token_type::KEYWORD_BREAK:
            return parse_break();

        case token_type::KEYWORD_CONTINUE:
            return parse_continue();

        case token_type::KEYWORD_LAMBDA:
            return parse_lambda();

        default: {
            token_class token {current_token()};
            return parse_expression_stmt(token); 
        }
    }
}

StmtPtr parser_class::parse_expression_stmt(const auto& token) {
    auto expr = parse_assignment();
    if (expr) {
        ExpressionStmt expr_stmt{token, std::move(expr)};
        auto expr_node = std::make_unique<StmtNode>(StmtNode{std::move(expr_stmt)});
        return expr_node;
    }

    return {};
}

ExprPtr parser_class::parse_self() {
    token_class token = current_token();
    consume(token_type::KEYWORD_SELF);

    std::unique_ptr<Identifier> attr;
    if (match(token_type::DOT)) {
        consume(token_type::DOT);
        if (!match(token_type::IDENTIFIER)) {
            debug_syntax_error();
        }
        attr = std::make_unique<Identifier>(current_token());
        consume();
    }

    SelfExpr self{token, std::move(attr)};
    auto self_node = std::make_unique<ExprNode>(ExprNode{std::move(self)});
    return self_node;
}

StmtPtr parser_class::parse_try() {
    token_class token = current_token();
    consume(token_type::KEYWORD_TRY);

    consume_newline();
    Block try_body = parse_block();

    std::unique_ptr<ExceptStmt> except_branch {nullptr};
    if (match(token_type::KEYWORD_EXCEPT)) {
        token_class except_token = current_token();
        consume(token_type::KEYWORD_EXCEPT);

        consume_newline();
        Block except_body = parse_block();

        except_branch = std::make_unique<ExceptStmt>(except_token, std::move(except_body));
    }

    std::unique_ptr<FinallyStmt> finally_branch {nullptr};
    if (match(token_type::KEYWORD_FINALLY)) {
        token_class finally_token = current_token();
        consume(token_type::KEYWORD_FINALLY);

        consume_newline();
        Block finally_body = parse_block();

        finally_branch = std::make_unique<FinallyStmt>(finally_token, std::move(finally_body));
    }

    std::unique_ptr<ElseStmt> else_branch {};
    if (match(token_type::KEYWORD_ELSE)) {
        token_class else_token = current_token();
        consume(token_type::KEYWORD_ELSE);

        consume_newline();
        Block else_body = parse_block();

        else_branch = std::make_unique<ElseStmt>(else_token, std::move(else_body));
    }

    TryStmt try_stmt{token, std::move(try_body), std::move(except_branch), std::move(finally_branch), std::move(else_branch)};
    auto try_node = std::make_unique<StmtNode>(StmtNode{std::move(try_stmt)});
    return try_node;
}

StmtPtr parser_class::parse_pass() {
    token_class token = current_token();
    consume(token_type::KEYWORD_PASS);

    PassStmt pass{token};
    auto pass_node = std::make_unique<StmtNode>(StmtNode{std::move(pass)});
    return pass_node;
}

ExprPtr parser_class::parse_assignment() {
    auto left = parse_logical_or();

    if (match(token_type::EQUAL)) {
        token_class token = current_token();
        consume(token_type::EQUAL);

        AssignmentOp assign{token, std::move(left), parse_assignment()};
        auto assign_node = std::make_unique<ExprNode>(ExprNode{std::move(assign)});
        return assign_node;
    } else if (match(token_type::PLUS_EQUAL) ||
               match(token_type::MINUS_EQUAL) ||
               match(token_type::STAR_EQUAL) ||
               match(token_type::SLASH_EQUAL)) {
        token_class op = current_token();
        consume(current_token().type);

        AugmentedAssignmentOp aug_assign{op, std::move(left), parse_assignment()};
        auto aug_assign_node = std::make_unique<ExprNode>(ExprNode{std::move(aug_assign)});
        return aug_assign_node;
    }

    return left;
}

StmtPtr parser_class::parse_return_stmt() {
    token_class token = current_token();
    consume(token_type::KEYWORD_RETURN);

    ExprPtr value;
    if (!is_at_end() && !match(token_type::NEWLINE)) {
        value = parse_expression_types();
    }

    ReturnStmt ret{token, std::move(value)};
    auto ret_node = std::make_unique<StmtNode>(StmtNode{std::move(ret)});
    return ret_node;
}

StmtPtr parser_class::parse_function_def() {
    consume(token_type::KEYWORD_DEF);

    token_class token = current_token();
    if (!match(token_type::IDENTIFIER)) {
        debug_syntax_error();
    }

    consume(token_type::IDENTIFIER);
    consume(token_type::LPAREN);

    ParameterList params;
    if (!match(token_type::RPAREN)) {
        if (match(token_type::IDENTIFIER)) {
            params.params.push_back(Parameter{current_token()});
            consume(token_type::IDENTIFIER);
        }

        while (match(token_type::COMMA)) {
            consume(token_type::COMMA);
            if (match(token_type::IDENTIFIER)) {
                params.params.push_back(Parameter{current_token()});
                consume(token_type::IDENTIFIER);
            }
        }
    }

    consume(token_type::RPAREN);
    consume(token_type::COLON);

    consume_line();
    Block body = parse_block();

    FunctionDef func{token, std::move(params), std::move(body)};
    auto func_node = std::make_unique<StmtNode>(StmtNode{std::move(func)});
    return func_node;
}

StmtPtr parser_class::parse_class() {
    consume(token_type::KEYWORD_CLASS);

    token_class token = current_token();
    consume(token_type::IDENTIFIER);

    consume(token_type::COLON);
    consume_line();

    Block body;
    body.token = current_token();

    while (!match(token_type::DEDENT) && !is_at_end()) {
        if (match(token_type::KEYWORD_DEF)) {
            auto method = parse_method();
            if (method) {
                body.statements.push_back(std::move(method));
            }
        } else {
            auto stmt = parse_statement();
            if (stmt) {
                body.statements.push_back(std::move(stmt));
            }
        }
    }

    consume(token_type::DEDENT);

    ClassDef cls{token, std::move(body)};
    auto cls_node = std::make_unique<StmtNode>(StmtNode{std::move(cls)});
    return cls_node;
}

StmtPtr parser_class::parse_method() {
    consume(token_type::KEYWORD_DEF);

    token_class token = current_token();
    if (!match(token_type::IDENTIFIER) && !match(token_type::KEYWORD_INIT)) {
        debug_syntax_error();
    }

    if (match(token_type::KEYWORD_INIT)) {
        consume(token_type::KEYWORD_INIT);
    } else {
        consume(token_type::IDENTIFIER);
    }

    valid_constructor = true; 

    consume(token_type::LPAREN);

    ParameterList params;
    if (!match(token_type::KEYWORD_SELF)) {
        debug_syntax_error();
    }

    params.params.push_back(Parameter{current_token()});
    consume(token_type::KEYWORD_SELF);

    while (match(token_type::COMMA)) {
        consume(token_type::COMMA);
        if (match(token_type::IDENTIFIER)) {
            params.params.push_back(Parameter{current_token()});
            consume(token_type::IDENTIFIER);
        }
    }

    consume(token_type::RPAREN);
    consume(token_type::COLON);

    consume_line();
    Block body = parse_block();

    MethodDef method{token, std::move(params), std::move(body)};
    auto method_node = std::make_unique<StmtNode>(StmtNode{std::move(method)});
    return method_node;
}

StmtPtr parser_class::parse_break() {
    token_class token = current_token();
    consume(token_type::KEYWORD_BREAK);

    BreakStmt brk{token};
    auto brk_node = std::make_unique<StmtNode>(StmtNode{std::move(brk)});
    return brk_node;
}

StmtPtr parser_class::parse_continue() {
    token_class token = current_token();
    consume(token_type::KEYWORD_CONTINUE);

    ContinueStmt cont{token};
    auto cont_node = std::make_unique<StmtNode>(StmtNode{std::move(cont)});
    return cont_node;
}

StmtPtr parser_class::parse_if_stmt() {
    token_class token = current_token();
    consume(token_type::KEYWORD_IF);

    auto condition = parse_logical_or();

    consume_newline();
    Block body = parse_block();

    std::vector<ElifStmt>  elifs {};
    while (match(token_type::KEYWORD_ELIF)) {
        token_class elif_token = current_token();
        consume(token_type::KEYWORD_ELIF);

        auto elif_condition = parse_expression_types();

        consume_newline();
        Block elif_body = parse_block();

        elifs.push_back(ElifStmt{elif_token, std::move(elif_condition), std::move(elif_body)});
    }

    std::unique_ptr<ElseStmt> else_branch;
    if (match(token_type::KEYWORD_ELSE)) {
        token_class else_token = current_token();
        consume(token_type::KEYWORD_ELSE);

        consume_newline();
        Block else_body = parse_block();

        else_branch = std::make_unique<ElseStmt>(else_token, std::move(else_body));
    }

    IfStmt if_stmt{token, std::move(condition), std::move(body), std::move(elifs), std::move(else_branch)};
    
    auto if_node = std::make_unique<StmtNode>(StmtNode{std::move(if_stmt)});
    
    return if_node;
}

StmtPtr parser_class::parse_while_stmt() {
    token_class token = current_token();
    consume(token_type::KEYWORD_WHILE);

    auto condition = parse_logical_or();

    consume_newline();
    Block body = parse_block();

    WhileStmt while_stmt{token, std::move(condition), std::move(body)};
    auto while_node = std::make_unique<StmtNode>(StmtNode{std::move(while_stmt)});
    return while_node;
}

StmtPtr parser_class::parse_for_stmt() {
    token_class token = current_token();
    consume(token_type::KEYWORD_FOR);

    Identifier variable{current_token()};
    if (match(token_type::IDENTIFIER)) {
        consume(token_type::IDENTIFIER);
    }

    consume(token_type::KEYWORD_IN);

    auto iterable = parse_expression_types();

    consume_newline();
    Block body = parse_block();

    ForStmt for_stmt{token, std::move(variable), std::move(iterable), std::move(body)};
    auto for_node = std::make_unique<StmtNode>(StmtNode{std::move(for_stmt)});
    return for_node;
}

StmtPtr parser_class::parse_match_stmt() {
    token_class token = current_token();
    consume(token_type::KEYWORD_MATCH);

    auto subject = parse_expression_types();

    consume_newline();
    std::vector<CaseStmt> cases;
    while (match(token_type::KEYWORD_CASE) && !is_at_end()) {
        token_class case_token = current_token();
        consume(token_type::KEYWORD_CASE);

        auto pattern = parse_expression_types();

        consume_newline();
        Block case_body = parse_block();

        cases.push_back(CaseStmt{case_token, std::move(pattern), std::move(case_body)});
    }

    consume(token_type::DEDENT);

    MatchStmt match_stmt{token, std::move(subject), std::move(cases)};
    auto match_node = std::make_unique<StmtNode>(StmtNode{std::move(match_stmt)});
    return match_node;
}

StmtPtr parser_class::parse_case() {
    token_class token = current_token();
    consume(token_type::KEYWORD_CASE);

    auto pattern = parse_expression_types();

    consume_newline();
    Block body = parse_block();

    CaseStmt case_stmt{token, std::move(pattern), std::move(body)};
    auto case_node = std::make_unique<StmtNode>(StmtNode{std::move(case_stmt)});
    return case_node;
}

ExprPtr parser_class::parse_list() {
    token_class token = current_token();
    consume(token_type::LBRACKET);

    ListExpr list{token, {}};

    if (match(token_type::RBRACKET)) {
        consume(token_type::RBRACKET);
        auto list_node = std::make_unique<ExprNode>(ExprNode{std::move(list)});
        return list_node;
    }

    list.elements.push_back(parse_expression_types());

    while (match(token_type::COMMA)) {
        consume(token_type::COMMA);

        if (match(token_type::RBRACKET)) {
            break;
        }

        list.elements.push_back(parse_expression_types());
    }

    consume(token_type::RBRACKET);
    auto list_node = std::make_unique<ExprNode>(ExprNode{std::move(list)});
    return list_node;
}

ExprPtr parser_class::parse_dict() {
    token_class token = current_token();
    consume(token_type::LCBRACE);

    DictExpr dict{token, {}};

    if (match(token_type::RCBRACE)) {
        consume(token_type::RCBRACE);
        auto dict_node = std::make_unique<ExprNode>(ExprNode{std::move(dict)});
        return dict_node;
    }

    auto key = parse_expression_types();

    while (match(token_type::COLON) || match(token_type::COMMA)) {
        consume(token_type::COLON);
        auto value = parse_expression_types();
        dict.entries.push_back({std::move(key), std::move(value)});

        if (match(token_type::COMMA)) {
            consume(token_type::COMMA);

            if (match(token_type::RCBRACE)) {
                break;
            }

            key = parse_expression_types();
        }
    }

    consume(token_type::RCBRACE);
    auto dict_node = std::make_unique<ExprNode>(ExprNode{std::move(dict)});
    return dict_node;
}

ExprPtr parser_class::parse_logical_and() {
    auto left = parse_equality();

    while (match(token_type::KEYWORD_AND)) {
        token_class op { current_token() };
        consume();

        AndOp and_op{op, std::move(left), parse_equality()};
        left = std::make_unique<ExprNode>(ExprNode{std::move(and_op)});
    }

    return left;
}

ExprPtr parser_class::parse_logical_or() {
    auto left = parse_logical_and();

    while (match(token_type::KEYWORD_OR)) {
        token_class op { current_token() };
        consume();

        OrOp or_op{op, std::move(left), parse_logical_and()};
        left = std::make_unique<ExprNode>(ExprNode{std::move(or_op)});
    }

    return left;
}

} 
