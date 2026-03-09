#include "backend/vm.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace TwoPy::Backend {
    VM::VM(const ByteCodeProgram& prgm) : m_prgm(prgm) {
        m_bp = m_prgm.chunks[0].get();
        m_instrutions = m_bp->code;
        m_frame_count = prgm.chunks.size();
    }

    VM::Result VM::run() {
        while (m_ip < m_instrutions.size()) {
            Instruction instr = m_instrutions[m_ip];
            m_ip++;
            try {
            switch (instr.opcode) {
                case OpCode::RETURN: {
                    return Result::OK;
                }

                /* Pushes to stack */
                case OpCode::LOAD_CONSTANT: {
                    vm_stack.push(m_bp->consts_pool[instr.argument]);
                    break;
                }

                case OpCode::ADD: {
                    Value rhs = vm_stack.top();
                    vm_stack.pop();

                    Value lhs = vm_stack.top();
                    vm_stack.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        vm_stack.push(Value(lhs.to_long() + rhs.to_long()));
                    } else {
                        vm_stack.push(Value(lhs.to_double() + rhs.to_double()));
                    }
                    break;
                }

                case OpCode::SUB: {
                    Value rhs = vm_stack.top();
                    vm_stack.pop();

                    Value lhs = vm_stack.top();
                    vm_stack.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        vm_stack.push(Value(lhs.to_long() - rhs.to_long()));
                    } else {
                        vm_stack.push(Value(lhs.to_double() - rhs.to_double()));
                    }
                    break;
                }

                case OpCode::MUL: {
                    Value rhs = vm_stack.top();
                    vm_stack.pop();

                    Value lhs = vm_stack.top();
                    vm_stack.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        vm_stack.push(Value(lhs.to_long() * rhs.to_long()));
                    } else {
                        vm_stack.push(Value(lhs.to_double() * rhs.to_double()));
                    }
                    break;
                }

                case OpCode::DIV: {
                    Value rhs = vm_stack.top();
                    vm_stack.pop();

                    Value lhs = vm_stack.top();
                    vm_stack.pop();

                    vm_stack.push(Value(lhs.to_double() / rhs.to_double()));
                    break;
                }

                /* gets rid of None Value */
                case OpCode::POP: {
                    vm_stack.pop();
                    break;
                }

                /* Pops from stack */
                case OpCode::STORE_NAME: {
                    auto name = vm_stack.top();
                    vm_stack.pop();

                    std::string key = m_bp->names_pool[instr.argument];
                    global_vars.insert_or_assign(key, name);
                    break;
                }

                /* Pushes to stack */
                case OpCode::LOAD_NAME: {
                    std::string var_name = m_bp->names_pool[instr.argument];
                    auto it = global_vars.find(var_name);
                    if (it != global_vars.end()) {
                        vm_stack.push(it->second);
                    } else if (var_name == "print") {
                        auto builtin = std::make_shared<FunctionPyObject>("print", std::vector<std::string>{}, 0);
                        vm_stack.push(Value(builtin));
                    }
                    break;
                }

                case OpCode::CALL_FUNCTION: {
                    std::uint8_t arg_count = instr.argument;

                    std::vector<Value> args(arg_count);
                    for (int i = arg_count - 1; i >= 0; i--) {
                        args[i] = vm_stack.top();
                        vm_stack.pop();
                    }

                    Value callable = vm_stack.top();
                    vm_stack.pop();

                    auto obj = callable.obj_ref();
                    if (auto* func = dynamic_cast<FunctionPyObject*>(obj.get())) {
                        if (func->name() == "print") {
                            for (std::size_t i = 0; i < args.size(); i++) {
                                if (i > 0) fmt::print(" ");
                                fmt::print("{}", args[i].to_string());
                            }
                            fmt::print("\n");
                            vm_stack.push(Value{});
                        }
                    }
                    break;
                }

                default:
                    break;
            }
            } catch (const std::exception& e) {
                throw std::runtime_error { fmt::format("lifetime error") };
            }
        }
        return Result::OK;
    }
}