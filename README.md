# TwoPy

A Python interpreter written in C++26. TwoPy takes Python source code through a full pipeline — lexing, parsing, bytecode compilation, and execution on a stack-based virtual machine.

Special thanks to DrkWithT for helping refactor the `match` and `consume` functions in the Parser namespace to use metaprogramming, eliminating verbose `consume(T) || consume(T)` chains for larger conditionals.

## Design

Based off a multi-pass compiler design. 

```
Python Source → Lexer → Tokens → Parser → AST → Compiler → Bytecode → VM → Output
```

- **Lexer**: Tokenizes Python source code. 
- **Parser**: Builds an AST via recursive descent with Pratt's parsing
- **Bytecode Compiler**: Compiles the AST into bytecode with constant/name pooling, scope-aware variable access, and jump patching
- **Stack-Based VM**: Executes bytecode with a global/local variable env with its own stack and instruction pointer.

### Supported Python Features

Currently only got basic arthemetic working for the vm and load/store ops.

The next test will be trying to get fizzbuzz working and fib. 

<img width="623" height="108" alt="Screenshot_20260308_202950" src="https://github.com/user-attachments/assets/7cd395f6-c356-4c2b-8e58-f09fda33e2d2" />

# TODO

For/while loops

List/Dict indexing 
