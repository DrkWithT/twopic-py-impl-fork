#pragma once

#include <cstdint>

namespace TwoPy::Backend {
    /*
    Opcode (add, subtract, jump, whatever)
    Vars/Constant (for your STORE_VARIABLE and LOAD_CONSTANT)
    */
    enum class OpCode : std::uint8_t {
        RETURN,
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
        BINARY_ADD,
        BINARY_SUB,

        STORE_FAST, // Local vars
        STORE_NAME, // Stuff like Classes, Functions, Dicts, Lists, etc etc

        COMPARE_OP, // != == < > 

        POP_JUMP_IF_FALSE, // AND stops if the first value is true
        POP_JUMP_IF_TRUE, // OR stops if the first value is true

        LOAD_FAST,  // Local vars
        LOAD_NAME,  // Module-level (mirrors STORE_NAME)
        LOAD_CONSTANT,
        
        JUMP_BACKWARD,  // While loops
    };

    /* Inside Python's bytecode 3.6 documentation. Use 2 bytes for each instruction. Previously the number of bytes varied by instruction.*/
    struct Instruction {
        OpCode opcode;          // VM opcode
        std::uint8_t argument;  // index to a certain constant or local variable slot
    };
}