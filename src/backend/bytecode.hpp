#ifndef TWOPY_BYTECODE_HPP 
#define TWOPY_BYTECODE_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "backend/value.hpp"
#include "frontend/ast.hpp"
#include "backend/opcode.hpp"

namespace TwoPy::Backend {
    struct FunctionChunk {
        std::vector<Instruction> code;
        std::vector<Value> consts_pool;
        std::vector<std::string> names_pool;
        std::size_t byte_offset; // instructions lists
        std::map<std::string, std::uint8_t> global_vars;
        std::map<std::string, std::uint8_t> local_vars;
        std::string name;
    };

    struct ByteCodeProgram {
        std::string name;
        std::vector<std::shared_ptr<FunctionChunk>> chunks;
    };

    class compiler {
    private:
        const TwoPy::Frontend::Program& m_program;
        std::size_t m_scope_depth;

        bool m_is_func_args = false;

        std::vector<std::size_t> pending_jumps;
        std::vector<std::size_t> truthy_jumps;

        FunctionChunk* m_curr_chunk {};

        ByteCodeProgram m_bytecode_program {};     

        // helper functions by https://craftinginterpreters.com/
        [[nodiscard]] std::size_t emit_jump(OpCode instruction) {
            m_curr_chunk->code.push_back({.opcode=instruction, .argument=0});
            m_curr_chunk->byte_offset += 2;
            return m_curr_chunk->byte_offset;
        }

        void patch_jump(std::size_t offset) {
            std::size_t jump_instr_index = (offset - 2) / 2; 
            m_curr_chunk->code[jump_instr_index].argument =
                static_cast<std::uint8_t>(m_curr_chunk->byte_offset);
        }      

        void emit_return_none() {
            auto it = std::ranges::find_if(m_curr_chunk->consts_pool, [](const Value& v) -> bool {
                return std::get_if<std::monostate>(&v.data()) != nullptr;
            });

            std::uint8_t none_index;
            if (it != m_curr_chunk->consts_pool.end()) {
                none_index = static_cast<std::uint8_t>(std::distance(m_curr_chunk->consts_pool.begin(), it));
            } else {
                none_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size());
                m_curr_chunk->consts_pool.emplace_back();
            }

            m_curr_chunk->code.push_back({.opcode=OpCode::LOAD_CONSTANT, .argument=none_index});
            m_curr_chunk->byte_offset += 2;
            
            m_curr_chunk->code.push_back({.opcode= (m_scope_depth > 0) ? OpCode::RETURN_VALUE : OpCode::RETURN});
            m_curr_chunk->byte_offset += 2;
        }

         /// TODO: I'll need to add detection for nested functions scoping
        void init_scope() {
            m_scope_depth++;
        }

        void end_scope() {
            m_scope_depth--;
        }

        void disassemble_body_while_stmt(const TwoPy::Frontend::Block& blk) {
            for (const auto& s : blk.statements) {
                disassemble_instruction(s);
            }
        }

        /* Non helper functions */
        void disassemble_instruction(const TwoPy::Frontend::StmtPtr& stmt);
        void disassemble_stmt(const TwoPy::Frontend::StmtNode& stmt);
        void disassemble_expr(const TwoPy::Frontend::ExprNode& expr);

        void disassemble_operators(const TwoPy::Frontend::OperatorsType& ops);
        void disassemble_literals(const TwoPy::Frontend::Literals& lits);
        void disassemble_literals_args(const TwoPy::Frontend::Literals& lits);

        void disassemble_function_object(const TwoPy::Frontend::FunctionDef& function);
        void disassemble_callexpr_object(const TwoPy::Frontend::CallExpr& callee);

        /* 
            * Jumping/Patching notes
        */
        void disassemble_elif_stmt(const TwoPy::Frontend::ElifStmt& stmt);
        void disassemble_if_stmt(const TwoPy::Frontend::IfStmt& stmt);
        void disassemble_while(const TwoPy::Frontend::WhileStmt& p_while);

        void disassemble_body_stmt(const TwoPy::Frontend::Block& blk);
        // pushing data to the stack
        void disassemble_identifier_expr(const TwoPy::Frontend::Identifier& iden);
        // popping data to the stack
        void disassemble_identifier_assignment_expr(const TwoPy::Frontend::Identifier& iden); 
        void disassemble_and_expr(const TwoPy::Frontend::AndOp& p_and);
        void disassemble_or_expr(const TwoPy::Frontend::OrOp& p_or);  
        
    public:
        compiler(const TwoPy::Frontend::Program& program);

        ByteCodeProgram disassemble_program();
        
        [[nodiscard]] std::optional<ByteCodeProgram> operator()();
    };
}

#endif