#include "backend/vm.hpp"

#include <print>
// #include <stdexcept>

namespace TwoPy::Backend {
    VM::VM(const ByteCodeProgram& prgm) : m_prgm(prgm) {
        m_bp = m_prgm.chunks[0].get();
        m_code = m_bp->code;
        m_frame_count = prgm.chunks.size();
    }

    Result VM::run() {
        // ! MUST-FIX: This loop may be incorrect, as the condition only assumes 1 chunk of bytecode is dispatched in a forward direction only. 
        // !What if control enters another function's chunk? What if we jump by a relative offset backwards? etc.
        while (m_ip < m_code.size() || m_frame_count > 0) {
            Instruction instr = m_code[m_ip];
            switch (instr.opcode) {
                case OpCode::RETURN: {
                    m_ip++;
                    m_frame_count -= 1;
                    break;
                }
                case OpCode::LOAD_CONSTANT: {
                    vm_stack.push(m_bp->consts_pool[instr.argument]);
                    m_ip++;
                    break;
                }
                case OpCode::ADD: {
                    Value rhs = vm_stack.top();
                    vm_stack.pop();

                    Value lhs = vm_stack.top();
                    vm_stack.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        vm_stack.emplace(lhs.to_long() + rhs.to_long());
                    } else {
                        vm_stack.emplace(lhs.to_double() + rhs.to_double());
                    }
                    m_ip++;
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
                    m_ip++;
                    break;
                }
                case OpCode::MUL: {
                    Value rhs = vm_stack.top();
                    vm_stack.pop();

                    Value lhs = vm_stack.top();
                    vm_stack.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        vm_stack.emplace(lhs.to_long() * rhs.to_long());
                    } else {
                        vm_stack.emplace(lhs.to_double() * rhs.to_double());
                    }
                    m_ip++;
                    break;
                }
                case OpCode::DIV: {
                    Value rhs = vm_stack.top();
                    vm_stack.pop();

                    Value lhs = vm_stack.top();
                    vm_stack.pop();

                    vm_stack.emplace(lhs.to_double() / rhs.to_double());
                    m_ip++;
                    break;
                }
                /* gets rid of None Value */
                case OpCode::POP: {
                    vm_stack.pop();
                    m_ip++;
                    break;
                }
                 /* Pops from stack */
                case OpCode::STORE_NAME: {
                    auto name = vm_stack.top();
                    vm_stack.pop();

                    const auto& key = m_bp->names_pool[instr.argument];
                    global_vars.insert_or_assign(key, name);
                    m_ip++;
                    break;
                }
                case OpCode::COMPARE_OP: {
                    const std::uint8_t cmp_id = m_code[m_ip].argument;

                    const Value lhs = vm_stack.top();
                    vm_stack.pop();

                    const Value rhs = vm_stack.top();
                    vm_stack.pop();

                    vm_stack.emplace(help_compare(cmp_id, lhs, rhs));

                    m_ip++;
                    break;
                }
                /* Pushes to stack */
                case OpCode::LOAD_NAME: {
                    const std::string& var_name = m_bp->names_pool[instr.argument];
                    
                    auto it = global_vars.find(var_name);
                    if (it != global_vars.end()) {
                        vm_stack.push(it->second);
                    } else if (var_name == "print") {
                        // ! MUST-FIX: Any built-in function should be resolved by its name at compile time / be cleanly looked up via global hashtable. A large if statement here would get messy later on.
                        auto builtin = std::make_shared<FunctionPyObject>("print", std::vector<std::string>{}, 0);
                        vm_stack.emplace(builtin);
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
                        args[i] = vm_stack.top();
                        vm_stack.pop();
                    }

                    Value callable = vm_stack.top();
                    vm_stack.pop();

                    auto obj = callable.obj_ref();
                    if (auto* func = dynamic_cast<FunctionPyObject*>(obj.get())) {
                        if (func->name() == "print") {
                            for (std::size_t i = 0; i < args.size(); i++) {
                                if (i > 0) std::print(" ");
                                std::print("{}", args[i].to_string());
                            }
                            std::print("\n");
                            vm_stack.emplace();
                        }
                    }
                    m_ip++;
                    break;
                }
                case OpCode::POP_JUMP_IF_FALSE: {
                    if (const auto& tos = vm_stack.top(); !tos.is_truthy()) {
                        vm_stack.pop();
                        m_ip = m_code[m_ip].argument;
                    } else {
                        vm_stack.pop();
                        m_ip++;
                    }
                    break;
                }
                case OpCode::POP_JUMP_IF_TRUE: {
                    if (const auto& tos = vm_stack.top(); tos.is_truthy()) {
                        vm_stack.pop();
                        m_ip = m_code[m_ip].argument;
                    } else {
                        vm_stack.pop();
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