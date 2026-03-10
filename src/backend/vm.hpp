#ifndef VM_HPP
#define VM_HPP

#include <stack>
#include <cstddef>
#include <vector>
#include <flat_map>
#include <string>

#include "backend/value.hpp"
#include "backend/bytecode.hpp"

/// NOTE: immutable accessor for impl. of __get__

// Instruction Pointer 
// Next instruction to execute
// Controls program flow

// Stack Pointer
// Top of the stack 
// Push/pop operations

// Base Pointer
// Start of the current stack frame
// Access local variables & arguments

namespace TwoPy::Backend {
    class VM {
    public:
        enum class Result : std::uint8_t {
            OK,
            RUNTIME_ERROR,
            COMPILER_ERROR,
        };
    private:
        const ByteCodeProgram& m_prgm {};
        
        std::size_t m_frame_count {};
        
        std::flat_map<std::string, Value> global_vars {};
        std::flat_map<std::string, Value> local_vars {};

        // stores runtime consts/values 
        std::stack<Value> vm_stack {};
        std::vector<Instruction> m_instrutions {};

        // Also called program counters 
        std::size_t m_ip {};

        // Base Pointer (EBP)
        Chunk* m_bp {};

    public:
        VM(const ByteCodeProgram& prgm);        

        Result run();
    };
}

#endif