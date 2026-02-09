#ifndef OPCODE_HPP 
#define OPCODE_HPP

#include <cstddef>
#include <vector>
#include <variant>
#include <deque>

#include "frontend/ast.hpp"
#include "backend/objects.hpp"

namespace TwoPyOpByteCode {
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

        STORE_VARIABLE,
        STORE_FAST, // Local vars

        LOAD_VARIABLE,
        LOAD_FAST, // Local vars
        LOAD_CONSTANT,
    };

    // Todo: Make an Object file which madles lists functions and code objects this will help with runtime vm

    //  Argument used as an index to map to a certain consts or vars pool 

    /* Inside Python's bytecode 3.6 documentation. Use 2 bytes for each instruction. Previously the number of bytes varied by instruction.*/
    struct ByteCode {
        OpCode opcode;
        std::uint8_t argument;
    };

    using Value = std::variant<std::monostate, long, double, std::string, TwoObject::RuntimeDetection>;

    /* This would be global bytecode lists */
    struct FullByteCode {
        std::vector<ByteCode> instructions;
        std::vector<Value> constants_pool;
        std::vector<std::string> vars_pool;
    };

    class chunk_class {
        private:
            FullByteCode m_global_bytecode {};
            //FullByteCode m_other_bytecode {};

            const Ast::Program& m_program;

            void disassemble_instruction(const Ast::StmtPtr& stmt);
            void disassemble_expr(const Ast::ExprNode& expr);
            void disassemble_stmt(const Ast::StmtNode& stmt);

        public:
            chunk_class(const Ast::Program& program);

            FullByteCode disassemble_program();
    };
}

#endif