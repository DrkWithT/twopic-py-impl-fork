#ifndef PYTHON_BYTE_HPP
#define PYTHON_BYTE_HPP

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <iomanip>

#include <fmt/core.h>
#include "backend/bytecode.hpp"
#include "backend/objects.hpp"
#include "backend/value.hpp"

namespace BytePrinter {
    using namespace TwoPy::Backend;

    inline std::string opcode_to_string(OpCode op) {
        switch (op) {
            case OpCode::RETURN: return "RETURN";
            case OpCode::CALL: return "CALL";
            case OpCode::PRINT: return "PRINT";
            case OpCode::ADD: return "ADD";
            case OpCode::SUB: return "SUB";
            case OpCode::MUL: return "MUL";
            case OpCode::DIV: return "DIV";
            case OpCode::POP: return "POP";
            case OpCode::PUSH: return "PUSH";
            case OpCode::MAKE_FUNCTION: return "MAKE_FUNCTION";
            case OpCode::CALL_FUNCTION: return "CALL_FUNCTION";
            case OpCode::PUSH_NULL: return "PUSH_NULL";
            case OpCode::BINARY_POWER: return "BINARY_POWER";
            case OpCode::BINARY_MODULO: return "BINARY_MODULO";
            case OpCode::STORE_FAST: return "STORE_FAST";
            case OpCode::STORE_NAME: return "STORE_NAME";
            case OpCode::COMPARE_OP: return "COMPARE_OP";
            case OpCode::POP_JUMP_IF_FALSE: return "POP_JUMP_IF_FALSE";
            case OpCode::POP_JUMP_IF_TRUE: return "POP_JUMP_IF_TRUE";
            case OpCode::LOAD_FAST: return "LOAD_FAST";
            case OpCode::LOAD_NAME: return "LOAD_NAME";
            case OpCode::LOAD_CONSTANT: return "LOAD_CONSTANT";
            default: return "UNKNOWN";
        }
    }

    inline std::string value_to_string(const Value& val) {
        const auto& data = val.data();

        if (std::holds_alternative<std::monostate>(data)) {
            return "None";
        } else if (std::holds_alternative<long>(data)) {
            return std::to_string(std::get<long>(data));
        } else if (std::holds_alternative<double>(data)) {
            return std::to_string(std::get<double>(data));
        } else if (std::holds_alternative<bool>(data)) {
            return std::get<bool>(data) ? "True" : "False";
        } else if (std::holds_alternative<Reference>(data)) {
            return "<ref>";
        } else if (std::holds_alternative<Value::py_object_ptr>(data)) {
            auto obj = std::get<Value::py_object_ptr>(data);
            if (!obj) {
                return "<null>";
            }
            if (obj->tag() == ObjectTag::STRING) {
                return "\"" + obj->stringify() + "\"";
            }
            return "<" + obj->stringify() + ">";
        }

        return "<unknown>";
    }

    inline void print_instruction(const Instruction& instr, size_t offset,
                                  const Chunk& chunk) {
        std::string opname = opcode_to_string(instr.opcode);

        fmt::print("{:>6}  {:<20}", offset * 2, opname);

        switch (instr.opcode) {
            case OpCode::LOAD_CONSTANT:
                if (instr.argument < chunk.consts_pool.size()) {
                    fmt::print(" {:>3}  ({})",
                              instr.argument,
                              value_to_string(chunk.consts_pool[instr.argument]));
                } else {
                    fmt::print(" {:>3}  <invalid constant index>", instr.argument);
                }
                break;

            case OpCode::STORE_FAST:
            case OpCode::LOAD_FAST:
            case OpCode::LOAD_NAME:
            case OpCode::STORE_NAME:
                if (instr.argument < chunk.names_pool.size()) {
                    fmt::print(" {:>3}  ({})",
                              instr.argument,
                              chunk.names_pool[instr.argument]);
                } else {
                    fmt::print(" {:>3}  <invalid variable index>", instr.argument);
                }
                break;

            case OpCode::POP_JUMP_IF_FALSE:
                fmt::print(" {:>3}  (to {})", (offset + 1) * 2, instr.argument);
                break;

            case OpCode::POP_JUMP_IF_TRUE:
                fmt::print(" {:>3}  (to {})", instr.argument / 2, instr.argument);
                break;

            case OpCode::CALL_FUNCTION:
            case OpCode::CALL:
                fmt::print(" {:>3}  (arg count)", instr.argument);
                break;

            default:
                if (instr.argument != 0) {
                    fmt::print(" {:>3}", instr.argument);
                }
                break;
        }

        fmt::print("\n");
    }

    inline void disassemble_chunk(const Chunk& chunk, const std::string& name = "<chunk>") {
        fmt::print("Disassembly of {}:\n", name);
        fmt::print("Constants: [");
        for (size_t i = 0; i < chunk.consts_pool.size(); ++i) {
            if (i > 0) fmt::print(", ");
            fmt::print("{}", value_to_string(chunk.consts_pool[i]));
        }
        fmt::print("]\n");

        fmt::print("Variables: [");
        for (size_t i = 0; i < chunk.names_pool.size(); ++i) {
            if (i > 0) fmt::print(", ");
            fmt::print("'{}'", chunk.names_pool[i]);
        }
        fmt::print("]\n\n");

        fmt::print("Offset  Opcode               Arg  Details\n");
        fmt::print("------  -------------------  ---  -------\n");

        for (size_t i = 0; i < chunk.code.size(); ++i) {
            print_instruction(chunk.code[i], i, chunk);
        }

        fmt::print("\n");
    }

    inline void disassemble_program(const ByteCodeProgram& program) {
        fmt::print("=== Bytecode Program: {} ===\n\n", program.name);

        for (size_t i = 0; i < program.chunks.size(); ++i) {
            std::string chunk_name = (i == 0) ? "<module>" : fmt::format("<chunk {}>", i);
            disassemble_chunk(*program.chunks[i], chunk_name);
        }

        fmt::print("=== End of {} ===\n", program.name);
    }
}

#endif
