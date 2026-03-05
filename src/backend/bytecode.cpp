#include "backend/bytecode.hpp"

#include <stdexcept>
#include <fmt/core.h>

/*
After doing some research, I found two ways of handling std::variant types
One would be to use std::hold_alternative which you'll have to manually check
or use std::visit which allows for more cleaner code in the future.
*/
namespace TwoPy::Backend {
    compiler::compiler(const TwoPy::Frontend::Program& program)
        : m_program(program), m_scope_depth(0) {
        m_bytecode_program.name = "<module>";
        auto module_chunk = std::make_shared<Chunk>();
        m_bytecode_program.chunks.push_back(module_chunk);
        m_curr_chunk = module_chunk;
    }

    ByteCodeProgram compiler::disassemble_program() {
        for (const auto& ptr : m_program.statements) {
            disassemble_instruction(ptr);
        }

        emit_return_none();
        return m_bytecode_program;
    }

    void compiler::disassemble_instruction(const TwoPy::Frontend::StmtPtr& stmt) {
        try {
            disassemble_stmt(*stmt);
        } catch (const std::exception& e) {
            fmt::print("Error: {}\n", e.what());
        }
    }

    void compiler::disassemble_stmt(const TwoPy::Frontend::StmtNode& stmt) {
        if (auto* expr_stmt = std::get_if<TwoPy::Frontend::ExpressionStmt>(&stmt.node)) {
            if (expr_stmt->expression) {
                disassemble_expr(*expr_stmt->expression);
            } else {
                throw std::runtime_error("Something went wrong");
            }
        }

        if (auto* func_def = std::get_if<TwoPy::Frontend::FunctionDef>(&stmt.node)) {
            disassemble_function_object(*func_def);
        }

        if (auto* if_stmt = std::get_if<TwoPy::Frontend::IfStmt>(&stmt.node)) {
             disassemble_if_stmt(*if_stmt);
        }

        if (auto* return_stmt = std::get_if<TwoPy::Frontend::ReturnStmt>(&stmt.node)) {
            if (return_stmt->value) {
                disassemble_expr(*return_stmt->value);
            } else {
                emit_return_none();
                return;
            }

            m_curr_chunk->code.push_back({OpCode::RETURN});
            m_curr_chunk->byte_offset += 2;
        }
    }

    void compiler::disassemble_body_stmt(const TwoPy::Frontend::Block& blk) {
        for (const auto& s : blk.statements) {
            disassemble_instruction(s);
            m_curr_chunk->code.push_back({OpCode::POP});
            m_curr_chunk->byte_offset += 2;
        }

        emit_return_none();
    }

    void compiler::disassemble_if_stmt(const TwoPy::Frontend::IfStmt& stmt) {
        disassemble_expr(*stmt.condition);

        auto jmp = emit_jump(OpCode::POP_JUMP_IF_FALSE);

        for (auto tj : truthy_jumps) {
            patch_jump(tj);
        }
        truthy_jumps.clear();

        disassemble_body_stmt(stmt.body);

        patch_jump(jmp);

        for (auto pj : pending_jumps) {
            patch_jump(pj);
        }
        pending_jumps.clear();

        if (stmt.else_branch != nullptr) {
            disassemble_body_stmt(stmt.else_branch->body);
        }

        for (const auto& stmt : stmt.elifs) {
            disassemble_elif_stmt(stmt);
        }
    }

    void compiler::disassemble_elif_stmt(const TwoPy::Frontend::ElifStmt& stmt) {
        disassemble_expr(*stmt.condition);

        auto jmp = emit_jump(OpCode::POP_JUMP_IF_FALSE);

        for (auto tj : truthy_jumps) {
            patch_jump(tj);
        }
        truthy_jumps.clear();

        disassemble_body_stmt(stmt.body);

        patch_jump(jmp);

        for (auto pj : pending_jumps) {
            patch_jump(pj);
        }
        pending_jumps.clear();
    }


    void compiler::disassemble_expr(const TwoPy::Frontend::ExprNode& expr) {
        if (auto* callee = std::get_if<TwoPy::Frontend::CallExpr>(&expr.node)) {
            disassemble_callexpr_object(*callee);
        }

         /// NOTE: Okay as DerekT Suggested Nested Variants is a horrible idea and should never be used in production! 
        /// I'm going to fix this inside my Ast.hpp where there will be no more nested variants. 
        if (auto* lits = std::get_if<TwoPy::Frontend::Literals>(&expr.node)) {
            disassemble_literals(*lits);
        }

        if (auto* ops = std::get_if<TwoPy::Frontend::OperatorsType>(&expr.node)) {
            disassemble_operators(*ops);
        }

        if (auto* ident = std::get_if<TwoPy::Frontend::Identifier>(&expr.node)) {
            disassemble_identifier_expr(*ident);
        }
    }

    void compiler::disassemble_identifier_expr(const TwoPy::Frontend::Identifier& iden) {
        std::uint8_t var_index;

        // if local 
        if (m_scope_depth > 0){
            if (!local_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                local_vars.insert({iden.token.value, var_index});
            } else {
                var_index = local_vars.at(iden.token.value);
            }

            m_curr_chunk->code.push_back({OpCode::LOAD_FAST, var_index});
            m_curr_chunk->byte_offset += 2;
        } else {
            if (!global_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                global_vars.insert({iden.token.value, var_index});
            } else {
                /// NOTE: .at() throws if it doesn't find the data
                var_index = global_vars.at(iden.token.value);
            }

            m_curr_chunk->code.push_back({OpCode::LOAD_NAME, var_index});
            m_curr_chunk->byte_offset += 2;
        }    
    }

    void compiler::disassemble_identifier_assignment_expr(const TwoPy::Frontend::Identifier& iden) {
        std::uint8_t var_index;

        // Locals
        if (m_scope_depth > 0) {
            if (!local_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                local_vars.insert({iden.token.value, var_index});
            } else {
                /// NOTE: .at() throws if it doesn't find the data
                var_index = local_vars.at(iden.token.value);
            }
            
            m_curr_chunk->code.push_back({OpCode::STORE_FAST, var_index});
            m_curr_chunk->byte_offset += 2;  
        } else { // Globals
            if (!global_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                global_vars.insert({iden.token.value, var_index});
            } else {
                /// NOTE: .at() throws if it doesn't find the data
                var_index = global_vars.at(iden.token.value);
            }
            
            m_curr_chunk->code.push_back({OpCode::STORE_NAME, var_index});
            m_curr_chunk->byte_offset += 2;  
        }
    }

    void compiler::disassemble_operators(const TwoPy::Frontend::OperatorsType& ops) {
        if (auto* assign = std::get_if<TwoPy::Frontend::AssignmentOp>(&ops)) {
            if (assign->value) {
                disassemble_expr(*assign->value);
            }

            if (assign->target) {
                if (auto* ident = std::get_if<TwoPy::Frontend::Identifier>(&assign->target->node)) {
                    disassemble_identifier_assignment_expr(*ident);
                }
            }
        }

        if (auto* term = std::get_if<TwoPy::Frontend::TermOp>(&ops)) {
            disassemble_expr(*term->left);
            disassemble_expr(*term->right);

            std::string op = term->op.value;
            if (op == "+") {
                m_curr_chunk->code.push_back({OpCode::ADD});
                m_curr_chunk->byte_offset += 2;
            } else if (op == "-") {
                m_curr_chunk->code.push_back({OpCode::SUB});
                m_curr_chunk->byte_offset += 2;
            }
            return;
        }

        if (auto* factor = std::get_if<TwoPy::Frontend::FactorOp>(&ops)) {
            disassemble_expr(*factor->left);
            disassemble_expr(*factor->right);

            std::string op = factor->op.value;
            if (op == "*") {
                m_curr_chunk->code.push_back({OpCode::MUL});
                m_curr_chunk->byte_offset += 2;
            } else if (op == "/") {
                m_curr_chunk->code.push_back({OpCode::DIV});
                m_curr_chunk->byte_offset += 2;
            } else if (op == "%") {
                m_curr_chunk->code.push_back({OpCode::BINARY_MODULO});
                m_curr_chunk->byte_offset += 2;
            } else if (op == "//") {
                m_curr_chunk->code.push_back({OpCode::BINARY_FLOOR_DIVIDE});
                m_curr_chunk->byte_offset += 2;
            }

            return;
        }

        if (auto* compare = std::get_if<TwoPy::Frontend::EqualityOp>(&ops)) {
            disassemble_expr(*compare->left);
            disassemble_expr(*compare->right);

            m_curr_chunk->code.push_back({OpCode::COMPARE_OP});
            m_curr_chunk->byte_offset += 2;

            return;
        } 
 
        if (auto* _and = std::get_if<TwoPy::Frontend::AndOp>(&ops)) {
            disassemble_and_expr(*_and);
        }

        if (auto* _or = std::get_if<TwoPy::Frontend::OrOp>(&ops)) {
            disassemble_or_expr(*_or);
        } 
    }

    void compiler::disassemble_literals(const TwoPy::Frontend::Literals& lits) {
        if (auto* int_lit = std::get_if<TwoPy::Frontend::IntegerLiteral>(&lits)) {
            m_curr_chunk->consts_pool.emplace_back(std::stol(int_lit->token.value));
            std::uint8_t const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

            m_curr_chunk->code.push_back({OpCode::LOAD_CONSTANT, const_index});
            m_curr_chunk->byte_offset += 2;
            return;
        }

        if (auto* float_lit = std::get_if<TwoPy::Frontend::FloatLiteral>(&lits)) {
            m_curr_chunk->consts_pool.emplace_back(std::stod(float_lit->token.value));
            std::uint8_t const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

            m_curr_chunk->code.push_back({OpCode::LOAD_CONSTANT, const_index});
            m_curr_chunk->byte_offset += 2;
            return;
        }

        if (auto* string_lit = std::get_if<TwoPy::Frontend::StringLiteral>(&lits)) {
            auto str_obj = std::make_shared<StringPyObject>(string_lit->token.value);

            m_curr_chunk->consts_pool.emplace_back(str_obj);

            std::uint8_t const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

            m_curr_chunk->code.push_back({OpCode::LOAD_CONSTANT, const_index});
            m_curr_chunk->byte_offset += 2;
            return;
        }

        if (auto* bool_lit = std::get_if<TwoPy::Frontend::BoolLiteral>(&lits)) {
            m_curr_chunk->consts_pool.emplace_back(bool_lit->token.value == "True");
            std::uint8_t const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

            m_curr_chunk->code.push_back({OpCode::LOAD_CONSTANT, const_index});
            m_curr_chunk->byte_offset += 2;

            return; 
        }
    }

    /// TODO: Since I'm Lazy, I forgot to add STORE/LOAD_FAST for local vars 
    void compiler::disassemble_function_object(const TwoPy::Frontend::FunctionDef& function) {
        auto func_chunk = std::make_shared<Chunk>();
        auto saved_chunk = std::make_shared<Chunk>();

        m_bytecode_program.chunks.push_back(func_chunk);
        std::uint8_t func_chunk_index = static_cast<std::uint8_t>(m_bytecode_program.chunks.size() - 1);

        /* I realize that shared_ptr doesn't copy the data and this only moves the ref counter
           but this would be a good habit that copying structs is a bad idea. */
        saved_chunk = std::move(m_curr_chunk);
        m_curr_chunk = std::move(func_chunk);

        init_scope();
        for (const auto& stmt : function.body.statements) {
            disassemble_instruction(stmt);
        }

        m_curr_chunk = std::move(saved_chunk);

        std::vector<std::string> param_names;
        for (const auto& param : function.params.params) {
            param_names.push_back(param.token.value);
        }

        auto func_obj = std::make_shared<FunctionPyObject>(
            function.token.value, std::move(param_names), func_chunk_index
        );
        end_scope();

        std::uint8_t code_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size());
        m_curr_chunk->consts_pool.emplace_back(func_obj);

        m_curr_chunk->code.push_back({OpCode::LOAD_CONSTANT, code_index});
        m_curr_chunk->byte_offset += 2;

        auto name_obj = std::make_shared<StringPyObject>(function.token.value);
        std::uint8_t name_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size());

        m_curr_chunk->consts_pool.emplace_back(name_obj);
        m_curr_chunk->code.push_back({OpCode::LOAD_CONSTANT, name_index});
        m_curr_chunk->byte_offset += 2;

        m_curr_chunk->code.push_back({OpCode::MAKE_FUNCTION, 0});
        m_curr_chunk->byte_offset += 2;

        std::uint8_t var_index;
        if (!global_vars.contains(function.token.value)) {
            m_curr_chunk->names_pool.push_back(function.token.value);
            var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
            global_vars.insert({function.token.value, var_index});
        } else {
            var_index = global_vars.at(function.token.value);
        }
        
        m_curr_chunk->code.push_back({OpCode::STORE_NAME, var_index});
        m_curr_chunk->byte_offset += 2;

        m_curr_chunk->consts_pool.emplace_back(name_obj);
    }

    void compiler::disassemble_callexpr_object(const TwoPy::Frontend::CallExpr& callee) {
        auto* ident = std::get_if<TwoPy::Frontend::Identifier>(&callee.callee->node);
        disassemble_identifier_expr(*ident);

        for (const auto& arg : callee.arguments) {
            disassemble_expr(*arg);
        }

        std::uint8_t arg_count = static_cast<std::uint8_t>(callee.arguments.size());
        m_curr_chunk->code.push_back({OpCode::CALL_FUNCTION, arg_count});
        m_curr_chunk->byte_offset += 2;
    }

    void compiler::disassemble_and_expr(const TwoPy::Frontend::AndOp& p_and) {
        disassemble_expr(*p_and.left);
        std::size_t and_jump = emit_jump(OpCode::POP_JUMP_IF_FALSE);
        pending_jumps.emplace_back(and_jump);

        disassemble_expr(*p_and.right);
    }

    void compiler::disassemble_or_expr(const TwoPy::Frontend::OrOp& p_or) {
        disassemble_expr(*p_or.left);
        std::size_t truthy_jump = emit_jump(OpCode::POP_JUMP_IF_TRUE);
        truthy_jumps.emplace_back(truthy_jump);

        disassemble_expr(*p_or.right);
    } 
}
