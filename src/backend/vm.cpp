#include "backend/vm.hpp"
#include "backend/bytecode.hpp"

#include <print>
#include <ranges>
// #include <stdexcept>

namespace TwoPy::Backend {
    VM::VM(const ByteCodeProgram& prgm) : m_prgm(prgm) {
        m_curr_bp = m_prgm.chunks[0].get();

        auto instructions = m_prgm.chunks
        | std::views::transform([](const auto& chunk) -> const std::vector<Instruction>& {
            return chunk->code;                       
        }) | std::views::join;

        m_code = instructions | std::ranges::to<std::vector<Instruction>>(); 
    }

   
    Result VM::run() {
        while (m_ip < m_code.size() || m_frame_count > 0) {
            Instruction instr = m_code[m_ip];
            switch (instr.opcode) {
                case OpCode::RETURN: {
                    m_ip++;
                    m_frame_count -= 1;
                    break;
                }
                case OpCode::LOAD_CONSTANT: {
                    value_stack_ptr.push(m_curr_bp->consts_pool[instr.argument]);
                    m_ip++;
                    break;
                }
                case OpCode::ADD: {
                    Value rhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    Value lhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        value_stack_ptr.emplace(lhs.to_long() + rhs.to_long());
                    } else {
                        value_stack_ptr.emplace(lhs.to_double() + rhs.to_double());
                    }
                    m_ip++;
                    break;
                }
                case OpCode::SUB: {
                    Value rhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    Value lhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        value_stack_ptr.push(Value(lhs.to_long() - rhs.to_long()));
                    } else {
                        value_stack_ptr.push(Value(lhs.to_double() - rhs.to_double()));
                    }
                    m_ip++;
                    break;
                }
                case OpCode::MUL: {
                    Value rhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    Value lhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        value_stack_ptr.emplace(lhs.to_long() * rhs.to_long());
                    } else {
                        value_stack_ptr.emplace(lhs.to_double() * rhs.to_double());
                    }
                    m_ip++;
                    break;
                }
                case OpCode::DIV: {
                    Value rhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    Value lhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    value_stack_ptr.emplace(lhs.to_double() / rhs.to_double());
                    m_ip++;
                    break;
                }
                /* gets rid of None Value */
                case OpCode::POP: {
                    value_stack_ptr.pop();
                    m_ip++;
                    break;
                }
                 /* Pops from stack */
                case OpCode::STORE_NAME: {
                    auto name = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    const auto& key = m_curr_bp->names_pool[instr.argument];
                    global_vars.insert_or_assign(key, name);
                    m_ip++;
                    break;
                }
                case OpCode::COMPARE_OP: {
                    const std::uint8_t cmp_id = m_code[m_ip].argument;

                    const Value lhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    const Value rhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    value_stack_ptr.emplace(help_compare(cmp_id, lhs, rhs));

                    m_ip++;
                    break;
                }
                /* Pushes to stack */
                case OpCode::LOAD_NAME: {
                    const std::string& var_name = m_curr_bp->names_pool[instr.argument];
                    
                    auto it = global_vars.find(var_name);
                    if (it != global_vars.end()) {
                        value_stack_ptr.push(it->second);
                    } else if (var_name == "print") {
                        // ! MUST-FIX: Any built-in function should be resolved by its name at compile time / be cleanly looked up via global hashtable. A large if statement here would get messy later on.
                        auto builtin = std::make_shared<FunctionPyObject>("print", std::vector<std::string>{}, 0);
                        value_stack_ptr.emplace(builtin);
                    } else {
                        return Result::RUNTIME_ERROR;
                    }

                    m_ip++;
                    break;
                }
                case OpCode::CALL_FUNCTION: {
                    std::uint8_t arg_count = instr.argument;

                    std::vector<Value> args(arg_count);
                    for (int i = arg_count - 1; i >= 0; i--) {
                        args[i] = value_stack_ptr.top();
                        value_stack_ptr.pop();
                    }

                    Value callable = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    auto obj = callable.obj_ref();
                    if (auto* func = dynamic_cast<FunctionPyObject*>(obj.get())) {
                        if (func->name() == "print") {
                            for (std::size_t i = 0; i < args.size(); i++) {
                                if (i > 0) std::print(" ");
                                std::print("{}", args[i].to_string());
                            }
                            std::print("\n");
                            value_stack_ptr.emplace();
                        }
                    }
                    m_ip++;
                    break;
                }
                case OpCode::POP_JUMP_IF_FALSE: {
                    if (const auto& tos = value_stack_ptr.top(); !tos.is_truthy()) {
                        value_stack_ptr.pop();
                        m_ip = m_code[m_ip].argument;
                    } else {
                        value_stack_ptr.pop();
                        m_ip++;
                    }
                    break;
                }
                case OpCode::POP_JUMP_IF_TRUE: {
                    if (const auto& tos = value_stack_ptr.top(); tos.is_truthy()) {
                        value_stack_ptr.pop();
                        m_ip = m_code[m_ip].argument;
                    } else {
                        value_stack_ptr.pop();
                        m_ip++;
                    }
                    break;
                }
                case OpCode::JUMP_BACKWARD: {
                    m_ip = m_code[m_ip].argument;
                    // TODO
                    break;
                }
                default: break;
            }
        }
        return Result::OK;
    }
}