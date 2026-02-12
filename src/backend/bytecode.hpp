#ifndef TWOPY_BYTECODE_HPP 
#define TWOPY_BYTECODE_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <map>

#include "backend/value.hpp"

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

        STORE_VARIABLE,
        STORE_FAST, // Local vars

        LOAD_VARIABLE,
        LOAD_FAST, // Local vars
        LOAD_CONSTANT,
    };

    /* Inside Python's bytecode 3.6 documentation. Use 2 bytes for each instruction. Previously the number of bytes varied by instruction.*/
    struct Instruction {
        OpCode opcode;          // VM opcode
        std::uint8_t argument;  // index to a certain constant or local variable slot
    };

    struct Chunk {
        std::vector<Instruction> code;
        std::vector<Value> consts;
    };

    struct Program {
        std::string name;
        std::vector<Chunk> chunks;
    };

    class Emitter {
    private:
        /// TODO: add state to track names to constants / local slots, build the bytecode chunks, etc.

    public:
        /// TODO: set up state to track names to constants / local slots, build the bytecode chunks, etc.
        Emitter();

        /// Add AST to bytecode methods here. 

        [[nodiscard]] std::optional<Program> operator()();
    };
}

#endif