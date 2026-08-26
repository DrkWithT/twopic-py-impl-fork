#include "backend/bytecode.hpp"
#include "frontend/ast.hpp"

#include <stdexcept>
#include <print>

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
        m_curr_chunk = module_chunk.get();
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
            std::print("Error: {}\n", e.what());
        }
    }

    void compiler::disassemble_stmt(const TwoPy::Frontend::StmtNode& stmt) {
        if (const auto* expr_stmt = std::get_if<TwoPy::Frontend::ExpressionStmt>(&stmt.node)) {
            if (expr_stmt->expression) {
                disassemble_expr(*expr_stmt->expression);
            } else {
                throw std::runtime_error("Something went wrong");
            }
        }

        if (const auto* func_def = std::get_if<TwoPy::Frontend::FunctionDef>(&stmt.node)) {
            disassemble_function_object(*func_def);
        }

        if (const auto* if_stmt = std::get_if<TwoPy::Frontend::IfStmt>(&stmt.node)) {
             disassemble_if_stmt(*if_stmt);
        }

        if (const auto* return_stmt = std::get_if<TwoPy::Frontend::ReturnStmt>(&stmt.node)) {
            if (return_stmt->value) {
                disassemble_expr(*return_stmt->value);
            } else {
                emit_return_none();
                return;
            }

            m_curr_chunk->code.push_back({.opcode=OpCode::RETURN});
            m_curr_chunk->byte_offset += 2;
        }

        if (const auto* while_stmt = std::get_if<TwoPy::Frontend::WhileStmt>(&stmt.node)) {
            disassemble_while(*while_stmt);
        }
    }

    void compiler::disassemble_body_stmt(const TwoPy::Frontend::Block& blk) {
        for (const auto& s : blk.statements) {
            disassemble_instruction(s);
            m_curr_chunk->code.push_back({.opcode=OpCode::POP});
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
        if (const auto* callee = std::get_if<TwoPy::Frontend::CallExpr>(&expr.node)) {
            disassemble_callexpr_object(*callee);
        }

        if (const auto* lits = std::get_if<TwoPy::Frontend::Literals>(&expr.node)) {
            disassemble_literals(*lits);
        }

        if (const auto* ops = std::get_if<TwoPy::Frontend::OperatorsType>(&expr.node)) {
            disassemble_operators(*ops);
        }

        if (const auto* ident = std::get_if<TwoPy::Frontend::Identifier>(&expr.node)) {
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

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_FAST, .argument=var_index});
            m_curr_chunk->byte_offset += 2;
        } else {
            if (!global_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                global_vars.insert({iden.token.value, var_index});
            } else {
                var_index = global_vars.at(iden.token.value);
            }

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_NAME, .argument=var_index});
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
                var_index = local_vars.at(iden.token.value);
            }
            
            m_curr_chunk->code.push_back({.opcode=OpCode::STORE_FAST, .argument=var_index});
            m_curr_chunk->byte_offset += 2;  
        } else { // Globals
            if (!global_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                global_vars.insert({iden.token.value, var_index});
            } else {
                var_index = global_vars.at(iden.token.value);
            }
            
            m_curr_chunk->code.push_back({.opcode=OpCode::STORE_NAME, .argument=var_index});
            m_curr_chunk->byte_offset += 2;  
        }
    }

    void compiler::disassemble_operators(const TwoPy::Frontend::OperatorsType& ops) {
        if (const auto* assign = std::get_if<TwoPy::Frontend::AssignmentOp>(&ops)) {
            if (assign->value) {
                disassemble_expr(*assign->value);
            }

            if (assign->target) {
                if (auto* ident = std::get_if<TwoPy::Frontend::Identifier>(&assign->target->node)) {
                    disassemble_identifier_assignment_expr(*ident);
                }
            }
        }

        if (const auto* term = std::get_if<TwoPy::Frontend::TermOp>(&ops)) {
            disassemble_expr(*term->left);
            disassemble_expr(*term->right);

            const auto& op = term->op.value;
            if (op == "+") {
                m_curr_chunk->code.push_back({.opcode=OpCode::ADD});
                m_curr_chunk->byte_offset += 2;
            } else if (op == "-") {
                m_curr_chunk->code.push_back({.opcode=OpCode::SUB});
                m_curr_chunk->byte_offset += 2;
            }
            return;
        }

        if (const auto* factor = std::get_if<TwoPy::Frontend::FactorOp>(&ops)) {
            disassemble_expr(*factor->left);
            disassemble_expr(*factor->right);

            const auto& op = factor->op.value;
            if (op == "*") {
                m_curr_chunk->code.push_back({.opcode=OpCode::MUL});
                m_curr_chunk->byte_offset += 2;
            } else if (op == "/") {
                m_curr_chunk->code.push_back({.opcode=OpCode::DIV});
                m_curr_chunk->byte_offset += 2;
            } else if (op == "%") {
                m_curr_chunk->code.push_back({.opcode=OpCode::BINARY_MODULO});
                m_curr_chunk->byte_offset += 2;
            } else if (op == "//") {
                m_curr_chunk->code.push_back({.opcode=OpCode::BINARY_FLOOR_DIVIDE});
                m_curr_chunk->byte_offset += 2;
            }

            return;
        }

        if (const auto* compare = std::get_if<TwoPy::Frontend::ComparisonOp>(&ops)) {
            disassemble_expr(*compare->left);
            disassemble_expr(*compare->right);

            m_curr_chunk->code.push_back({.opcode=OpCode::COMPARE_OP});
            m_curr_chunk->byte_offset += 2; 
           
            return;
        }

        if (const auto* compare = std::get_if<TwoPy::Frontend::EqualityOp>(&ops)) {
            disassemble_expr(*compare->left);
            disassemble_expr(*compare->right);

            m_curr_chunk->code.push_back({.opcode=OpCode::COMPARE_OP});
            m_curr_chunk->byte_offset += 2;

            return;
        } 
 
        if (const auto* _and = std::get_if<TwoPy::Frontend::AndOp>(&ops)) {
            disassemble_and_expr(*_and);
        }

        if (const auto* _or = std::get_if<TwoPy::Frontend::OrOp>(&ops)) {
            disassemble_or_expr(*_or);
        } 
    }

    void compiler::disassemble_literals(const TwoPy::Frontend::Literals& lits) {
        if (const auto* int_lit = std::get_if<TwoPy::Frontend::IntegerLiteral>(&lits)) {
            m_curr_chunk->consts_pool.emplace_back(std::stol(int_lit->token.value));
            const auto const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_CONSTANT, .argument=const_index});
            m_curr_chunk->byte_offset += 2;
            return;
        }

        if (const auto* float_lit = std::get_if<TwoPy::Frontend::FloatLiteral>(&lits)) {
            m_curr_chunk->consts_pool.emplace_back(std::stod(float_lit->token.value));
            auto const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_CONSTANT, .argument=const_index});
            m_curr_chunk->byte_offset += 2;
            return;
        }

        if (const auto* string_lit = std::get_if<TwoPy::Frontend::StringLiteral>(&lits)) {
            auto str_obj = std::make_shared<StringPyObject>(string_lit->token.value);

            m_curr_chunk->consts_pool.emplace_back(str_obj);

            const auto const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_CONSTANT, .argument=const_index});
            m_curr_chunk->byte_offset += 2;
            return;
        }

        if (const auto* bool_lit = std::get_if<TwoPy::Frontend::BoolLiteral>(&lits)) {
            m_curr_chunk->consts_pool.emplace_back(bool_lit->token.value == "True");
            auto const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_CONSTANT, .argument=const_index});
            m_curr_chunk->byte_offset += 2;

            return; 
        }
    }

    /// TODO: Since I'm Lazy, I forgot to add STORE/LOAD_FAST for local vars 
    void compiler::disassemble_function_object(const TwoPy::Frontend::FunctionDef& function) {
        auto func_chunk = std::make_shared<Chunk>();

        m_bytecode_program.chunks.push_back(func_chunk);
        auto func_chunk_index = static_cast<std::uint8_t>(m_bytecode_program.chunks.size() - 1);

        /* 
            ! old way of using shared_ptr had a cost 
        */
        Chunk* saved_chunk = m_curr_chunk;
        m_curr_chunk = func_chunk.get(); 

        init_scope();
        for (const auto& stmt : function.body.statements) {
            disassemble_instruction(stmt);
        }

        m_curr_chunk = saved_chunk;

        std::vector<std::string> param_names;
        param_names.reserve(function.params.params.size());
        for (const auto& param : function.params.params) {
            param_names.push_back(param.token.value);
        }

        auto func_obj = std::make_shared<FunctionPyObject>(
            function.token.value, std::move(param_names), func_chunk_index
        );
        end_scope();

        auto code_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size());
        m_curr_chunk->consts_pool.emplace_back(func_obj);

        m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_CONSTANT, .argument=code_index});
        m_curr_chunk->byte_offset += 2;

        auto name_obj = std::make_shared<StringPyObject>(function.token.value);
        const auto name_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size());

        m_curr_chunk->consts_pool.emplace_back(name_obj);
        m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_CONSTANT, .argument=name_index});
        m_curr_chunk->byte_offset += 2;

        m_curr_chunk->code.push_back({.opcode=OpCode::MAKE_FUNCTION, .argument=0});
        m_curr_chunk->byte_offset += 2;

        std::uint8_t var_index;
        if (!global_vars.contains(function.token.value)) {
            m_curr_chunk->names_pool.push_back(function.token.value);
            var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
            global_vars.insert({function.token.value, var_index});
        } else {
            var_index = global_vars.at(function.token.value);
        }
        
        m_curr_chunk->code.push_back({.opcode=OpCode::STORE_NAME, .argument=var_index});
        m_curr_chunk->byte_offset += 2;

        m_curr_chunk->consts_pool.emplace_back(name_obj);
    }

    void compiler::disassemble_callexpr_object(const TwoPy::Frontend::CallExpr& callee) {
        auto* ident = std::get_if<TwoPy::Frontend::Identifier>(&callee.callee->node);
        disassemble_identifier_expr(*ident);

        for (const auto& arg : callee.arguments) {
            disassemble_expr(*arg);
        }

        auto arg_count = static_cast<std::uint8_t>(callee.arguments.size());
        m_curr_chunk->code.push_back({.opcode=OpCode::CALL_FUNCTION, .argument=arg_count});
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

    void compiler::disassemble_while(const TwoPy::Frontend::WhileStmt& p_while) {
        auto start = static_cast<std::uint8_t>(m_curr_chunk->byte_offset);
        disassemble_expr(*p_while.condition);

        auto jmp = emit_jump(OpCode::POP_JUMP_IF_FALSE);

        for (auto tj : truthy_jumps) {
            patch_jump(tj);
        }
        truthy_jumps.clear();

        disassemble_body_while_stmt(p_while.body);

        m_curr_chunk->code.push_back({.opcode=OpCode::JUMP_BACKWARD, .argument=start});
        m_curr_chunk->byte_offset += 2;
        
        patch_jump(jmp);

        for (auto pj : pending_jumps) {
            patch_jump(pj);
        }
        pending_jumps.clear();
    }
}
