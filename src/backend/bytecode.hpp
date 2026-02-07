#ifndef OPCODE_HPP 
#define OPCODE_HPP

#include <cstddef>
#include <vector>
#include <variant>

#include "frontend/ast.hpp"

namespace TwoPyOpByteCode {
    /* 
    Opcode (add, subtract, jump, whatever)
    Offset (used for JUMP / JUMP_IF_TRUE )
    Literal/Constant (for your STORE_VARIABLE and LOAD_CONSTANT)
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
        STORE_VARIABLE,
        LOAD_VARIABLE,
        LOAD_CONSTANT
    }

    struct ByteCode {
        OpCode opcode;
        std::uint8_t argument;
    };

    using ByteCodeTotal = std::vector<ByteCode>;
    
}

#endif