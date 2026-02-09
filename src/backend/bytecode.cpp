#include "backend/bytecode.hpp"

#include <fmt/core.h>

using namespace TwoPyOpByteCode;

/*
After doing some research, I found two ways of handling std::variant types
One would be to use std::hold_alternative which you'll have to manually check
or use std::visit which allows for more cleaner code in the futre.
*/

chunk_class::chunk_class(const Ast::Program& program)
    : m_program(program) {}

FullByteCode chunk_class::disassemble_program() {
    for (const auto& stmt : m_program.statements) {
        disassemble_instruction(stmt);
    }
    return m_global_bytecode;
}

void chunk_class::disassemble_instruction(const Ast::StmtPtr& stmt) {
    try {
        disassemble_stmt(*stmt);
    } catch (const std::exception& e) {
        fmt::print(stderr, "Error: {}\n", e.what());
    }
}   

void chunk_class::disassemble_stmt(const Ast::StmtNode& stmt) {
    if (std::holds_alternative<Ast::ExpressionStmt>(stmt.node)) {
        auto& expr_stmt = std::get<Ast::ExpressionStmt>(stmt.node);
        disassemble_expr(*expr_stmt.expression);
    } else if (std::holds_alternative<Ast::FunctionDef>(stmt.node)) {
        auto& func_stmt = std::get<Ast::FunctionDef>(stmt.node);
    } else if (std::holds_alternative<Ast::ReturnStmt>(stmt.node)) {
        auto& return_stmt = std::get<Ast::ReturnStmt>(stmt.node);
        m_global_bytecode.instructions.push_back({OpCode::RETURN});
    }
}

void chunk_class::disassemble_expr(const Ast::ExprNode& expr) {
    if (std::holds_alternative<Ast::Literals>(expr.node)) {
        auto& lits = std::get<Ast::Literals>(expr.node);
        if (std::holds_alternative<Ast::IntegerLiteral>(lits)) {
            auto& integer = std::get<Ast::IntegerLiteral>(lits);
            m_global_bytecode.constants_pool.push_back(std::stol(integer.token.value));
            m_global_bytecode.instructions.push_back({
                OpCode::LOAD_CONSTANT,
                static_cast<std::uint8_t>(m_global_bytecode.constants_pool.size() - 1)
            });
        }

    } else if (std::holds_alternative<Ast::OperatorsType>(expr.node)) {
        auto& ops = std::get<Ast::OperatorsType>(expr.node);
        if (std::holds_alternative<Ast::AssignmentOp>(ops)) {
            auto& assign = std::get<Ast::AssignmentOp>(ops);
            disassemble_expr(*assign.value);
            auto* ident = std::get_if<Ast::Identifier>(&assign.target->node);
            if (ident) {
                m_global_bytecode.vars_pool.push_back(ident->token.value);
                m_global_bytecode.instructions.push_back({
                    OpCode::STORE_VARIABLE,
                    static_cast<std::uint8_t>(m_global_bytecode.vars_pool.size() - 1)
                });
            }
        }
    }
}
