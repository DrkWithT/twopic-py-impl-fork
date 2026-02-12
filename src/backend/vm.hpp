#ifndef VM_HPP
#define VM_HPP

#include <vector>
#include "backend/value.hpp"
#include "backend/bytecode.hpp"

namespace TwoPy::Backend {
    class VM {
    private:
        // 1. Make a value stack, call frame stack, stack index, instruction pointer, status code, and more...
        // 2. To start, pretend that PyFunctions are just readonly wrappers around a pointer into a Chunk? The VM just needs to enter its instruction pointer into that function code. Getting it working is 1st priority.

    public:
        /// TODO: initialize state mentioned above with program??
        VM(const Program& prgm);

        /// TODO: implement opcodes & run.
        
        /// NOTE: runs the dispatch loop on the program's bytecode & returns true if execution was OK :)
        [[nodiscard]] bool run();
    };
}

#endif