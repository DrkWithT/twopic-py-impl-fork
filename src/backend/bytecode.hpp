#ifndef TWOPY_BYTECODE_HPP 
#define TWOPY_BYTECODE_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <ranges>
#include <algorithm>
#include <any>

#include "backend/value.hpp"
#include "frontend/ast.hpp"

namespace TwoPy::Backend {
    /*
    Opcode (add, subtract, jump, whatever)
    Vars/Constant (for your STORE_VARIABLE and LOAD_CONSTANT)
    */
    enum class OpCode : std::uint8_t {
        RETURN,
        CALL,
        PRINT,
        ADD,
        SUB,
        MUL,
        DIV,
        POP,
        PUSH,

        MAKE_FUNCTION,
        CALL_FUNCTION,
        PUSH_NULL,		// Prepares the stack for a function call.

        BINARY_POWER,
        BINARY_MODULO,
        BINARY_FLOOR_DIVIDE,

        STORE_FAST, // Local vars
        STORE_NAME, // Stuff like Classes, Functions, Dicts, Lists, etc etc

        COMPARE_OP, // != ==

        POP_JUMP_IF_FALSE, // AND stops if the first value is true
        POP_JUMP_IF_TRUE, // OR stops if the first value is true

        LOAD_FAST,  // Local vars
        LOAD_NAME,  // Module-level (mirrors STORE_NAME)
        LOAD_CONSTANT,
    };

    /* Inside Python's bytecode 3.6 documentation. Use 2 bytes for each instruction. Previously the number of bytes varied by instruction.*/
    struct Instruction {
        OpCode opcode;          // VM opcode
        std::uint8_t argument;  // index to a certain constant or local variable slot
    };

    struct Chunk {
        std::vector<Instruction> code;
        std::vector<Value> consts_pool;
        std::vector<std::string> names_pool;
        std::size_t byte_offset; // instructions lists
    };

    struct ByteCodeProgram {
        std::string name;
        std::vector<std::shared_ptr<Chunk>> chunks;
    };

    class compiler {
    private:
        const TwoPy::Frontend::Program& m_program;

        bool is_in_function = false;

        std::map<std::string, std::uint8_t> global_vars {};
        std::vector<std::size_t> m_pending_jumps {};
        std::vector<std::size_t> m_truthy_jumps {};

        std::shared_ptr<Chunk> m_curr_chunk {};

        ByteCodeProgram m_bytecode_program {};     

        // helper functions by https://craftinginterpreters.com/
        [[nodiscard]] std::size_t emit_jump(OpCode instruction) {
            m_curr_chunk->code.push_back({instruction, 0});
            m_curr_chunk->byte_offset += 2;
            return m_curr_chunk->byte_offset;
        }

        void patch_jump(std::size_t offset) {
            std::size_t jump_instr_index = (offset - 2) / 2; 
            m_curr_chunk->code[jump_instr_index].argument =
                static_cast<std::uint8_t>(m_curr_chunk->byte_offset);
        }      

        void emit_return_none() {
            auto it = std::ranges::find_if(m_curr_chunk->consts_pool, [](const Value& v) {
                return std::get_if<std::monostate>(&v.data()) != nullptr;
            });

            std::uint8_t none_index;
            if (it != m_curr_chunk->consts_pool.end()) {
                none_index = static_cast<std::uint8_t>(std::distance(m_curr_chunk->consts_pool.begin(), it));
            } else {
                none_index = static_cast<std::uint8_t>(m_curr_chunk->consts_pool.size());
                m_curr_chunk->consts_pool.emplace_back();
            }

            m_curr_chunk->code.push_back({OpCode::LOAD_CONSTANT, none_index});
            m_curr_chunk->byte_offset += 2;

            m_curr_chunk->code.push_back({OpCode::RETURN});
            m_curr_chunk->byte_offset += 2;
        }

        void add_chunk_code(OpCode code, std::uint8_t arg = 0) {
            m_curr_chunk->code.push_back({code, arg});            
        }

        void disassemble_instruction(const TwoPy::Frontend::StmtPtr& stmt);
        void disassemble_stmt(const TwoPy::Frontend::StmtNode& stmt);
        void disassemble_expr(const TwoPy::Frontend::ExprNode& expr);

        void disassemble_operators(const TwoPy::Frontend::OperatorsType& ops);
        void disassemble_literals(const TwoPy::Frontend::Literals& lits);

        void disassemble_function_object(const TwoPy::Frontend::FunctionDef& function);
        void disassemble_callexpr_object(const TwoPy::Frontend::CallExpr& callee);
        void disassemble_elif_stmt(const TwoPy::Frontend::ElifStmt& stmt);
        void disassemble_if_stmt(const TwoPy::Frontend::IfStmt& stmt);
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