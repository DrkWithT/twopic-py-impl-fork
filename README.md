# TwoPy

A toy Python interpreter written in C++26.

## Usage
 - Ensure you have CMake 3.20+ and a C++26 supporting compiler.
 - Enable `helper.sh`.
 - Do `./helper.sh help` for usage info. Use `./helper.sh build debug-build <your build tool>` to create the binary.

## Design

This is based off typical compiler stages:
- **Lexer**: Tokenizes Python source code.
- **Parser**: Builds an AST via recursive descent with Pratt's parsing
- **Bytecode Compiler**: Compiles the AST into bytecode with constant/name pooling, scope-aware variable access, and jump patching
- **Stack-Based VM**: Executes bytecode with a global/local variable env with its own stack and instruction pointer.

## Supported Python Features (around 3.8 - 3.9)
 - Basic variable store/load, comparisons, arithmetic.
 - print??

<img width="623" height="108" alt="Screenshot_20260308_202950" src="https://github.com/user-attachments/assets/7cd395f6-c356-4c2b-8e58-f09fda33e2d2" />

## Contributors

Special thanks to DrkWithT for helping refactor the `match` and `consume` functions in the Parser namespace to use metaprogramming, eliminating verbose `consume(T) || consume(T)` chains for larger conditionals.

## TODO
 - _VM Refactor 1_
    - New stack representation supporting arbitrary peeking (presized vector with SP, BP)
    - Stack of call frames for recursion support
 - Get Fibonacci working as microbenchmark 1
 - Support nested functions
 - For loops
 - Ternary expressions?
 - Support lists
 - Support dictionaries
