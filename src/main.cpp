#include <fstream>
#include <sstream>
#include <string_view>
#include <string>
#include <fmt/core.h>

#include "frontend/lexical.hpp"
#include "frontend/parser.hpp"
#include "print/ast_tree.hpp"   
#include "print/python_byte.hpp"
#include "backend/bytecode.hpp"
#include "backend/vm.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fmt::print(stderr, "Usage: {} <file.py>\n", argv[0]);
        fmt::print(stderr, "Example: {} test.py\n", argv[0]);
        return 1;
    }

    try {
        const std::string& source_code = TwoPy::Frontend::read_file(argv[1]);
        TwoPy::Frontend::lexical_class lexer(source_code);

        TwoPy::Frontend::parser_class parser(lexer);
        TwoPy::Frontend::Program program = parser.parse();


        TwoPy::Backend::compiler bytecode_compiler(program);
        TwoPy::Backend::ByteCodeProgram bytecode_program = bytecode_compiler.disassemble_program();

        // BytePrinter::disassemble_program(bytecode_program);

        TwoPy::Backend::VM py_vm(bytecode_program);

        py_vm.run();

        // fmt::print("\n=== ABSTRACT SYNTAX TREE ===\n");

        // AstPrinter::print_ast(program);

        // fmt::print("\n=== PARSING COMPLETE ===\n"); 
    
    } catch (const std::exception& e) {
        fmt::print(stderr, "Error: {}\n", e.what());
        return 1;
    }

    return 0;
}
