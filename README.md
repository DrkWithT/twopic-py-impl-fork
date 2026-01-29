# Lexical Analyzer

A Python lexical analyzer and parser written in C++23 that tokenizes Python source code and builds an Abstract Syntax Tree (AST).

Special Thanks to DrkWithT with helping me refactoring the match and consume inside the Parser namespace to include metaprogramming. This way I don't have to call consume(T) || consume (T) for larger conditionals.

## Features

- **Lexical Analysis**: Tokenizes Python source code into meaningful tokens
- **Syntax Parsing**: Builds an AST using recursive descent parsing
- **Python Language Support**:
  - Function definitions (`def`)
  - Class definitions (`class`)
  - Control flow (`if`, `elif`, `else`, `while`, `for`)
  - Pattern matching (`match`/`case`)
  - Binary operators (`+`, `-`, `*`, `/`, `//`, `%`, `**`)
  - Assignment operators (`=`, `+=`, `-=`, `*=`, `/=`)
  - Literals (integers, floats, strings)
  - Return statements

## Building

### Prerequisites

- CMake 3.10 or higher
- C++23 compatible compiler (GCC 11+, Clang 14+)

### Build Instructions

```bash
cmake -B build
cmake --build build
```

The executable will be located at `build/bin/lexical_analyzer`.

## Usage

```bash
./build/bin/lexical_analyzer <python_file>
```

### Example

```bash
./build/bin/lexical_analyzer example.py
```

This will:
1. Tokenize the input Python file
2. Parse the tokens into an AST
3. Display the AST structure

## Project Structure

```
lexical/
├── src/
│   ├── lexical.hpp      # Lexer interface
│   ├── lexical.cpp      # Lexer implementation
│   ├── parser.hpp       # Parser interface
│   ├── parser.cpp       # Parser implementation
│   ├── ast.hpp          # AST node definitions
│   └── token.hpp        # Token type definitions
├── main.cpp             # Entry point
├── CMakeLists.txt       # Build configuration
└── example.py           # Example Python file
```

## AST Structure

The parser generates an AST with the following node types:

- `PROGRAM` - Root node
- `FUNCTION_DEF` - Function definitions
- `CLASS_DEF` - Class definitions
- `IF_STMT`, `ELIF_STMT`, `ELSE_STMT` - Conditional statements
- `WHILE_STMT`, `FOR_STMT` - Loop statements
- `MATCH_STMT`, `CASE_STMT` - Pattern matching
- `ASSIGNMENT` - Variable assignments
- `BINARY_OP` - Binary operations
- `RETURN_STMT` - Return statements
- `IDENTIFIER` - Variable/function names
- `INTEGER_LITERAL`, `FLOAT_LITERAL`, `STRING_LITERAL` - Literals

## Example Output

For the following Python code:

```python
def greet(name):
    message = "Hello, " + name
    return message
```

The parser generates:

```
Node: PROGRAM
  Node: FUNCTION_DEF (value: "greet")
    Node: PARAMETER_LIST
      Node: PARAMETER (value: "name")
    Node: BLOCK
      Node: ASSIGNMENT (value: "=")
        Node: IDENTIFIER (value: "message")
        Node: BINARY_OP (value: "+")
          Node: STRING_LITERAL (value: "Hello, ")
          Node: IDENTIFIER (value: "name")
      Node: RETURN_STMT
        Node: IDENTIFIER (value: "message")
```

## Implementation Details

### Recursive Descent Parsing

The parser uses recursive descent with the following precedence hierarchy:

1. `parse_statement()` - Statement dispatcher
2. `parse_assignment()` - Assignment expressions
3. `parse_operator()` - Binary operators
4. `parse_expression()` - Primary expressions
5. Specialized parsers for control structures

### Token Types

The lexer recognizes:
- Keywords: `def`, `class`, `if`, `while`, `for`, `match`, `case`, `return`, etc.
- Operators: `+`, `-`, `*`, `/`, `//`, `%`, `**`, `=`, `+=`, etc.
- Literals: integers, floats, strings
- Delimiters: `()`, `[]`, `{}`, `:`, `,`
- Indentation: `INDENT`, `DEDENT`, `NEWLINE`

## License

This project is provided as-is for educational purposes.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.
