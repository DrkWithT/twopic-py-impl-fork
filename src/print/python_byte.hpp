#ifndef PYTHON_BYTE_HPP
#define PYTHON_BYTE_HPP

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <iomanip>
#include <algorithm>

#include <print>
#include <format>
#include "backend/bytecode.hpp"
#include "backend/objects.hpp"
#include "backend/value.hpp"

namespace BytePrinter {
    using namespace TwoPy::Backend;

    inline std::string opcode_to_string(OpCode op) {
        switch (op) {
            case OpCode::RETURN: return "RETURN";
            case OpCode::RETURN_VALUE: return "RETURN_VALUE";
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
            case OpCode::BINARY_FLOOR_DIVIDE: return "BINARY_FLOOR_DIVIDE";
            case OpCode::BINARY_ADD: return "BINARY_ADD";
            case OpCode::BINARY_SUB: return "BINARY_SUB";
            case OpCode::LOAD_SMALL_INT: return "LOAD_SMALL_INT";
            case OpCode::STORE_FAST: return "STORE_FAST";
            case OpCode::STORE_NAME: return "STORE_NAME";
            case OpCode::COMPARE_OP: return "COMPARE_OP";
            case OpCode::POP_JUMP_IF_FALSE: return "POP_JUMP_IF_FALSE";
            case OpCode::POP_JUMP_IF_TRUE: return "POP_JUMP_IF_TRUE";
            case OpCode::LOAD_FAST: return "LOAD_FAST";
            case OpCode::LOAD_NAME: return "LOAD_NAME";
            case OpCode::LOAD_CONSTANT: return "LOAD_CONSTANT";
            case OpCode::JUMP_BACKWARD: return "JUMP_BACKWARD";
        }

        // No default case: -Wswitch flags an opcode added to the enum but not named here.
        return std::format("UNKNOWN({})", static_cast<unsigned>(op));
    }

    inline bool is_jump(OpCode op) {
        switch (op) {
            case OpCode::POP_JUMP_IF_FALSE:
            case OpCode::POP_JUMP_IF_TRUE:
            case OpCode::JUMP_BACKWARD:
                return true;
            default:
                return false;
        }
    }

    /* Jump arguments hold the absolute byte offset of their target (see compiler::patch_jump). */
    inline std::vector<std::size_t> collect_jump_targets(const FunctionChunk& chunk) {
        std::vector<std::size_t> targets;
        for (const auto& instr : chunk.code) {
            if (is_jump(instr.opcode)) {
                targets.push_back(instr.argument);
            }
        }
        return targets;
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

    /* Renders the arg column plus a "(to N)" detail for an absolute jump target. */
    inline std::string jump_argument_string(const Instruction& instr, size_t offset,
                                            const FunctionChunk& chunk) {
        const std::size_t target = instr.argument;
        const std::size_t here = offset * 2;
        const std::size_t code_size = chunk.code.size() * 2;

        std::string detail = std::format(" {:>3}  (to {}", instr.argument, target);

        if (target > code_size) {
            detail += ", past end of chunk";
        } else if (target % 2 != 0) {
            detail += ", misaligned";
        } else if (target == here) {
            detail += ", self loop";
        }

        return detail + ")";
    }

    inline std::string instruction_argument_string(const Instruction& instr, size_t offset,
                                                   const FunctionChunk& chunk) {
        switch (instr.opcode) {
            case OpCode::LOAD_CONSTANT:
                if (instr.argument < chunk.consts_pool.size()) {
                    return std::format(" {:>3}  ({})",
                                       instr.argument,
                                       value_to_string(chunk.consts_pool[instr.argument]));
                }
                return std::format(" {:>3}  <invalid constant index>", instr.argument);

            // Locals are slot-indexed, so their names come from local_vars
            // (CPython prints these from co_varnames, not co_names).
            case OpCode::STORE_FAST:
            case OpCode::LOAD_FAST:
                for (const auto& [name, slot] : chunk.local_vars) {
                    if (slot == instr.argument) {
                        return std::format(" {:>3}  ({})", instr.argument, name);
                    }
                }
                return std::format(" {:>3}  <invalid local slot>", instr.argument);

            case OpCode::LOAD_NAME:
            case OpCode::STORE_NAME:
                if (instr.argument < chunk.names_pool.size()) {
                    return std::format(" {:>3}  ({})",
                                       instr.argument,
                                       chunk.names_pool[instr.argument]);
                }
                return std::format(" {:>3}  <invalid variable index>", instr.argument);

            case OpCode::POP_JUMP_IF_FALSE:
            case OpCode::POP_JUMP_IF_TRUE:
            case OpCode::JUMP_BACKWARD:
                return jump_argument_string(instr, offset, chunk);

            case OpCode::CALL_FUNCTION:
                return std::format(" {:>3}  (arg count)", instr.argument);

            case OpCode::COMPARE_OP:
                // The compiler does not encode which comparison yet, so there is nothing to name.
                return "      (operator not encoded)";

            default:
                if (instr.argument != 0) {
                    return std::format(" {:>3}", instr.argument);
                }
                return {};
        }
    }

    inline void print_instruction(const Instruction& instr, size_t offset,
                                  const FunctionChunk& chunk,
                                  const std::vector<std::size_t>& jump_targets = {}) {
        const std::string opname = opcode_to_string(instr.opcode);
        const std::string detail = instruction_argument_string(instr, offset, chunk);

        // CPython's dis marks jump destinations with ">>" so loop bodies are easy to spot.
        const bool is_target = std::ranges::find(jump_targets, offset * 2) != jump_targets.end();

        if (detail.empty()) {
            std::print("{:<2}{:>6}  {}\n", is_target ? ">>" : "", offset * 2, opname);
        } else {
            std::print("{:<2}{:>6}  {:<20}{}\n", is_target ? ">>" : "", offset * 2, opname, detail);
        }
    }

    inline void disassemble_chunk(const FunctionChunk& chunk, const std::string& name = "<chunk>") {
        std::print("Disassembly of {}:\n", name);
        std::print("Constants: [");
        for (size_t i = 0; i < chunk.consts_pool.size(); ++i) {
            if (i > 0) std::print(", ");
            std::print("{}", value_to_string(chunk.consts_pool[i]));
        }
        std::print("]\n");

        std::print("Variables: [");
        for (size_t i = 0; i < chunk.names_pool.size(); ++i) {
            if (i > 0) std::print(", ");
            std::print("'{}'", chunk.names_pool[i]);
        }
        std::print("]\n");

        std::print("Bytes: {} ({} instructions)\n\n", chunk.code.size() * 2, chunk.code.size());

        const auto jump_targets = collect_jump_targets(chunk);

        std::print("  Offset  Opcode               Arg  Details\n");
        std::print("  ------  -------------------  ---  -------\n");

        for (size_t i = 0; i < chunk.code.size(); ++i) {
            print_instruction(chunk.code[i], i, chunk, jump_targets);
        }

        std::print("\n");
    }

    inline void disassemble_program(const ByteCodeProgram& program) {
        std::print("=== Bytecode Program: {} ===\n\n", program.name);

        for (size_t i = 0; i < program.chunks.size(); ++i) {
            std::string chunk_name = (i == 0) ? "<module>" : std::format("<chunk {}>", i);
            disassemble_chunk(*program.chunks[i], chunk_name);
        }

        std::print("=== End of {} ===\n", program.name);
    }
}

#endif
