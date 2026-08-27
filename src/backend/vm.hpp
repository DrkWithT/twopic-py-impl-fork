#ifndef VM_HPP
#define VM_HPP

#include <optional>
#include <stack>
#include <cstddef>
#include <vector>
#include <flat_map>
#include <string>
#include <map>

#include "backend/opcode.hpp"
#include "backend/value.hpp"
#include "backend/bytecode.hpp"

/* 
    ! Notes 

    The old vm was wrong where it took in a stack of Value. This isn't how a proper call stack where the stack takes in callframes.
    I haven't handled recursion yet. 
*/

namespace TwoPy::Backend {
    // ! Don't start on CallFrame until call_function
    struct CallFrame {
        std::size_t caller_bp;
        std::optional<std::size_t> callee_bp; // ! start of func args 
        Instruction* caller_ip;
        std::size_t frame_id;
        std::map<std::string, Value> vars;
    };

    enum class Result : std::uint8_t {
        OK,
        RUNTIME_ERROR,
        COMPILER_ERROR,
    };

    class VM {
        private:
            const ByteCodeProgram& m_prgm {};
            
            std::size_t m_frame_count {};

            std::stack<CallFrame> frames;
            
            std::flat_map<std::string, Value> global_vars {};

            std::stack<Value> value_stack_ptr {};
            std::vector<Instruction> m_code {};

            // Also called program counters 
            std::size_t m_ip {};

            // Base Pointer (EBP)
            Chunk* m_curr_bp {};

            [[nodiscard]]
            constexpr bool help_compare(std::uint8_t cmp_id, const Value& lhs, const Value& rhs) {
                switch (cmp_id) {
                    case 0: return lhs < rhs; // <
                    case 1: return !(lhs > rhs); // <=
                    case 2: return lhs == rhs; // ==
                    case 3: return lhs != rhs; // !=
                    case 4: return lhs > rhs; // >
                    case 5: default: return !(lhs < rhs); // >=
                }
            }

        public:
            VM(const ByteCodeProgram& prgm);        

            Result run();
    };

   
}

#endif
