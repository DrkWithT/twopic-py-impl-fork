#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexical.hpp"
#include "token.hpp"

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <python_file.py>\n";
        std::cerr << "Example: ./lexical_analyzer example.py\n";
        return 1;
    }

    std::string filename = argv[1];

    try {
        std::string source_code = read_file(filename);

        std::cout << "Analyzing file: " << filename << "\n";
        std::cout << "File size: " << source_code.size() << " bytes\n";
        std::cout << "-----------------------------------\n\n";

        // Create lexical analyzer
        Lexical::lexical_class lexer(source_code);

        // TODO: Once you implement the tokenize() method:
        // auto tokens = lexer.tokenize();
        //
        // for (const auto& token : tokens) {
        //     std::cout << "Token: " << token.type << " | Value: " << token.value << "\n";
        // }

        std::cout << "Lexical analysis complete!\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
