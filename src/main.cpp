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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fmt::print(stderr, "Usage: {} <file.py>\n", argv[0]);
        fmt::print(stderr, "Example: {} test.py\n", argv[0]);
        return 1;
    }

    fmt::print("=== Parsing file: {} ===\n\n", argv[1]);

    try {
        const std::string& source_code = Lexical::read_file(argv[1]);
        Lexical::lexical_class lexer(source_code);

        Parser::parser_class parser(lexer);
        Ast::Program program = parser.parse();

        fmt::print("\n=== ByteCode Syntax ===\n");
        
        TwoPyOpByteCode::chunk_class values(program);
        TwoPyOpByteCode::FullByteCode bytecode = values.disassemble_program();

        BytePrinter::print_bytecode(bytecode);

        // actually this is useless but I'll add it
        fmt::print("size of total python bytecode, {}\n", bytecode.instructions.size());

        fmt::print("\n=== ByteCode End ===\n");

       /*  fmt::print("\n=== ABSTRACT SYNTAX TREE ===\n");

        AstPrinter::print_ast(program);

        fmt::print("\n=== PARSING COMPLETE ===\n"); */

    } catch (const std::exception& e) {
        fmt::print(stderr, "Error: {}\n", e.what());
        return 1;
    }

    return 0;
}
