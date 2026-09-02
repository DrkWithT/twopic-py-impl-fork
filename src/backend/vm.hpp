#ifndef VM_HPP
#define VM_HPP

#include <stack>
#include <cstddef>
#include <vector>
#include <flat_map>
#include <string>

#include "backend/opcode.hpp"
#include "backend/value.hpp"
#include "backend/bytecode.hpp"

/* 
    ! Notes 

    The old vm was wrong where it took in a stack of Value. This isn't how a proper call stack where the stack takes in callframes.
    I haven't handled recursion yet. 

    Currently I don't have it where the main function is part of the call frame
*/

namespace TwoPy::Backend {
    // ? local vars are set up in a queue. 
    struct CallFrame {
        std::size_t caller_bp; // * outside function ex: main()
        std::size_t caller_ip {};
        std::flat_map<std::string, Value> local_vars {};
        FunctionChunk* chunk;
    };
    
    enum class Result : std::uint8_t {
        OK,
        RUNTIME_ERROR,
        COMPILER_ERROR,
    };

    class VM {
        private:
            const ByteCodeProgram& m_prgm {};
            std::vector<FunctionChunk> m_all_chunks;
            std::stack<CallFrame> m_frames;
            
            std::flat_map<std::string, Value> global_vars {};

            std::stack<Value> value_stack_ptr;
            std::vector<Instruction> m_module_code;

            // Also called program counters 
            std::size_t m_ip {};

            // Base Pointer (EBP)
            FunctionChunk* m_module_bp {};
            CallFrame* m_curr_frame_ptr {};

            [[nodiscard]]
            static constexpr bool help_compare(std::uint8_t cmp_id, const Value& lhs, const Value& rhs) {
                switch (cmp_id) {
                    case 0: return lhs < rhs; // <
                    case 1: return !(lhs > rhs); // <=
                    case 2: return lhs == rhs; // ==
                    case 3: return lhs != rhs; // !=
                    case 4: return lhs > rhs; // >
                    default: return !(lhs < rhs); // >=
                }
            }

            // ! Derek if you don't like the name you can change it
            constexpr void empty_call_frames() {
                if (m_frames.empty()) {
                    m_ip++;
                } else {
                    m_curr_frame_ptr->caller_ip += 1;
                }
            }

        public:
            VM(const ByteCodeProgram& prgm);        

            Result run();
    };
}

#endif
