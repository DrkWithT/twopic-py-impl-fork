#include <string_view>
#include <string>
#include <fmt/core.h>

#include "frontend/lexical.hpp"
#include "frontend/parser.hpp"
#include "print/ast_tree.hpp"   
#include "print/python_byte.hpp"
#include "backend/bytecode.hpp"
#include "backend/vm.hpp"

void show_usage(const char* process_path) {
    fmt::print(stderr, "Usage: {} [-a | -d | -r] <file.py>\n\t-d: dump bytecode\n", process_path);
    fmt::print(stderr, "Example: {} test.py\n", process_path);
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        show_usage(argv[0]);
        return 1;
    }

    std::string_view option_str {argv[1]};
    std::string file_path {argv[2]};
    const auto allow_ast_dump = option_str == "-a";
    const auto allow_bytecode_dump = option_str == "-d";
    const auto allow_run = option_str == "-r";

    if (!allow_ast_dump && !allow_bytecode_dump && !allow_run) {
        show_usage(argv[0]);
        return 1;
    }

    try {
        const std::string& source_code = TwoPy::Frontend::read_file(file_path);
        TwoPy::Frontend::lexical_class lexer(source_code);

        TwoPy::Frontend::parser_class parser(lexer);
        TwoPy::Frontend::Program program = parser.parse();

        if (allow_ast_dump) {
            fmt::print("\n=== ABSTRACT SYNTAX TREE ===\n");
            AstPrinter::print_ast(program);
            return 0;
        }

        TwoPy::Backend::compiler bytecode_compiler(program);
        TwoPy::Backend::ByteCodeProgram bytecode_program = bytecode_compiler.disassemble_program();

        if (allow_bytecode_dump) {
            fmt::print("\n=== BYTECODE ===\n");
            BytePrinter::disassemble_program(bytecode_program);
            return 0;
        }

        if (!allow_run) {
            return 0;
        }

        TwoPy::Backend::VM py_vm(bytecode_program);
        auto result = py_vm.run();

        if (result == TwoPy::Backend::VM::Result::RUNTIME_ERROR) {
            throw std::runtime_error("You need more logic");
        } else {
            fmt::print("logic good");
        }
    } catch (const std::exception& e) {
        fmt::print(stderr, "Error: {}\n", e.what());
        return 1;
    }

    return 0;
}
