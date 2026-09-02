#include "backend/bytecode.hpp"
#include "backend/objects.hpp"
#include "backend/opcode.hpp"
#include "frontend/ast.hpp"

#include <memory>
#include <ranges>
#include <optional>
#include <stdexcept>
#include <print>

namespace TwoPy::Backend {
    compiler::compiler(const TwoPy::Frontend::Program& program)
        : m_program(program), m_scope_depth(0) {
        m_bytecode_program.name = "<module>";
        auto module_chunk = std::make_shared<FunctionChunk>();
        module_chunk->name = "<module>";
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

            emit_return_none();
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
            if (m_is_func_args) {
                disassemble_literals_args(*lits);
            } else {
                disassemble_literals(*lits);
            }
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
            if (!m_curr_chunk->local_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                m_curr_chunk->local_vars.insert({iden.token.value, var_index});
            } else {
                var_index = m_curr_chunk->local_vars.at(iden.token.value);
            }

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_FAST, .argument=var_index});
            m_curr_chunk->byte_offset += 2;
        } else {
            if (!m_curr_chunk->global_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                m_curr_chunk->global_vars.insert({iden.token.value, var_index});
            } else {
                var_index = m_curr_chunk->global_vars.at(iden.token.value);
            }

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_NAME, .argument=var_index});
            m_curr_chunk->byte_offset += 2;
        }    
    }

    void compiler::disassemble_identifier_assignment_expr(const TwoPy::Frontend::Identifier& iden) {
        std::uint8_t var_index;

        // Locals
        if (m_scope_depth > 0) {
            if (!m_curr_chunk->local_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                m_curr_chunk->local_vars.insert({iden.token.value, var_index});
            } else {
                var_index = m_curr_chunk->local_vars.at(iden.token.value);
            }
            
            m_curr_chunk->code.push_back({.opcode=OpCode::STORE_FAST, .argument=var_index});
            m_curr_chunk->byte_offset += 2;  
        } else { // Globals
            if (!m_curr_chunk->global_vars.contains(iden.token.value)) {
                m_curr_chunk->names_pool.push_back(iden.token.value);
                var_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
                m_curr_chunk->global_vars.insert({iden.token.value, var_index});
            } else {
                var_index = m_curr_chunk->global_vars.at(iden.token.value);
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
    
    void compiler::disassemble_literals_args(const TwoPy::Frontend::Literals& lits) {
          if (const auto* int_lit = std::get_if<TwoPy::Frontend::IntegerLiteral>(&lits)) {
            const auto token_value = std::stol(int_lit->token.value);

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_SMALL_INT, .argument=static_cast<std::uint8_t>(token_value)});
            m_curr_chunk->byte_offset += 2;
            return;
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

    /**  
        * @note Python has where it makes an ObjectBase then it changes it to a FunctionPyObject at runtime with MAKE_FUNCTION
        * @note There is a section in 3.10 where MAKE_FUNCTION has some attribute flags based on the function params type values. 
    */
    void compiler::disassemble_function_object(const TwoPy::Frontend::FunctionDef& function) {
        std::shared_ptr<ObjectBase> obj;
        Value obj_value(obj);

        m_curr_chunk->consts_pool.emplace_back(obj_value);

        auto const_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size() - 1);

        m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_CONSTANT, .argument=const_index});
        m_curr_chunk->byte_offset += 2;      

        /* Init function block */
        auto func_chunk = std::make_shared<FunctionChunk>();
        FunctionChunk* saved_chunk = m_curr_chunk;
        m_curr_chunk = func_chunk.get();

        init_scope();
        for (const auto& stmt : function.body.statements) {
            disassemble_instruction(stmt);
        }
        emit_return_none();

        m_curr_chunk = saved_chunk;
        end_scope();
        /* End Function Block */

        auto param_names = function.params.params 
        | std::views::transform([](const auto& p) -> std::string {
            return p.token.value; 
        }) | std::ranges::to<std::vector>();
        
        func_chunk->name = function.token.value;
        m_bytecode_program.chunks.push_back(func_chunk);
        auto func_chunk_index = static_cast<std::uint8_t>(m_bytecode_program.chunks.size() - 1);

        obj = std::make_shared<FunctionPyObject>(
            function.token.value, std::move(param_names), func_chunk_index
        );        
        
        /* Backpatch the placeholder const reserved above, now that the chunk index is known. */
        m_curr_chunk->consts_pool[const_index] = Value {obj};

        m_curr_chunk->code.push_back({.opcode=OpCode::MAKE_FUNCTION});
        m_curr_chunk->byte_offset += 2; 
      
        m_curr_chunk->names_pool.push_back(function.token.value);
        auto func_index = static_cast<std::uint8_t>(m_curr_chunk->names_pool.size() - 1);
        m_curr_chunk->code.push_back({.opcode=OpCode::STORE_NAME, .argument=func_index});
        m_curr_chunk->byte_offset += 2; 
    }

    void compiler::disassemble_callexpr_object(const TwoPy::Frontend::CallExpr& callee) {
        const auto* ident = std::get_if<TwoPy::Frontend::Identifier>(&callee.callee->node);
        disassemble_identifier_expr(*ident);

        m_curr_chunk->code.push_back({.opcode=OpCode::PUSH_NULL});
        m_curr_chunk->byte_offset += 2;

        m_is_func_args = true;
        for (const auto& arg : callee.arguments) {
            disassemble_expr(*arg);
        }
        m_is_func_args = false;

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
