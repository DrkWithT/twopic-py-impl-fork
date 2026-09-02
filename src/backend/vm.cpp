#include "backend/bytecode.hpp"
#include "backend/opcode.hpp"
#include "backend/value.hpp"
#include "backend/vm.hpp"

#include <variant>
// #include <stdexcept>

namespace TwoPy::Backend {
    VM::VM(const ByteCodeProgram& prgm) : m_prgm(prgm) {
        std::vector<FunctionChunk> chunks;
        chunks.reserve(m_prgm.chunks.size());

        for (const auto& p : m_prgm.chunks) {
            chunks.push_back(std::move(*p));
        }

        m_all_chunks = std::move(chunks);

        m_module_bp = m_all_chunks.data();       
        m_module_code = m_module_bp->code;
    }
   
    Result VM::run() {
        while (m_ip < m_module_code.size()) {
            Instruction instr;

            if (m_frames.empty()) {
                instr = m_module_code[m_ip];
            } else {
                auto ip = m_curr_frame_ptr->caller_ip;
                instr = m_curr_frame_ptr->chunk->code[ip];    
            }

            switch (instr.opcode) {
                case OpCode::RETURN: {
                    m_ip++;
                    break;
                }
                case OpCode::LOAD_CONSTANT: {
                    value_stack_ptr.emplace(m_module_bp->consts_pool[instr.argument]);
                
                    empty_call_frames();
                    break;
                }
                case OpCode::LOAD_SMALL_INT: {
                    Value value (static_cast<long>(instr.argument));
                    value_stack_ptr.emplace(value);
                    
                    empty_call_frames();
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

                    empty_call_frames();
                    break;
                }
                case OpCode::RETURN_VALUE: {         
                    m_frames.pop();
                    m_curr_frame_ptr = nullptr;
                    break;
                }   
                case OpCode::LOAD_FAST: {
                    const std::string& var_name = m_curr_frame_ptr->chunk->names_pool[instr.argument];

                    auto it = m_curr_frame_ptr->local_vars.find(var_name);
                    if (it != m_curr_frame_ptr->local_vars.end()) {
                        value_stack_ptr.emplace(it->second);
                    } else {
                        return Result::RUNTIME_ERROR;
                    }

                    empty_call_frames();
                    break;
                }
                case OpCode::STORE_FAST: {
                    const auto value = value_stack_ptr.top();
                    value_stack_ptr.pop();
                    const auto& key =   m_curr_frame_ptr->chunk->names_pool[instr.argument];
                    m_curr_frame_ptr->local_vars.insert_or_assign(key, value);

                    empty_call_frames();
                    break;
                }
                case OpCode::SUB: {
                    Value rhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    Value lhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    if (std::holds_alternative<long>(lhs.data()) && std::holds_alternative<long>(rhs.data())) {
                        value_stack_ptr.emplace(lhs.to_long() - rhs.to_long());
                    } else {
                        value_stack_ptr.emplace(lhs.to_double() - rhs.to_double());
                    }

                    empty_call_frames();
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

                    empty_call_frames();
                    break;
                }
                case OpCode::DIV: {
                    Value rhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    Value lhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    value_stack_ptr.emplace(lhs.to_double() / rhs.to_double());

                    empty_call_frames();
                    break;
                }
                /* gets rid of None Value */
                case OpCode::POP: {
                    value_stack_ptr.pop();
                    empty_call_frames();

                    break;
                }
                 /* Pops from stack */
                case OpCode::STORE_NAME: {                    
                    const auto value = value_stack_ptr.top();
                    value_stack_ptr.pop();
                    const auto& key = m_module_bp->names_pool[instr.argument];
                    global_vars.insert_or_assign(key, value);

                    empty_call_frames();
                    break;
                }
                case OpCode::MAKE_FUNCTION: {
                    empty_call_frames();
                    break;
                }
                case OpCode::COMPARE_OP: {
                    const std::uint8_t cmp_id = m_module_code[m_ip].argument;

                    const Value lhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    const Value rhs = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    value_stack_ptr.emplace(help_compare(cmp_id, lhs, rhs));

                    empty_call_frames();
                    break;
                }
                /* Pushes to stack */
                case OpCode::LOAD_NAME: {
                    // ! Function block
                    const std::string& var_name = m_module_bp->names_pool[instr.argument];

                    auto it = global_vars.find(var_name);
                    if (it != global_vars.end()) {
                        value_stack_ptr.push(it->second);
                    }  else {
                        return Result::RUNTIME_ERROR;
                    }

                    empty_call_frames();
                    break;
                }
                /* 
                    ! Purpose: NULL for non-method and vise versa
                */
                case OpCode::PUSH_NULL: {
                    Value nil {std::monostate{}};
                    value_stack_ptr.emplace(nil);

                    empty_call_frames();
                    break;
                }
                case OpCode::CALL_FUNCTION: {
                    std::deque<Value> local_values;
                    std::uint8_t curr_arg_size = 0;
                    while (curr_arg_size < instr.argument) {
                        Value popped_value = value_stack_ptr.top();
                        value_stack_ptr.pop();

                        local_values.emplace_front(popped_value);
                        curr_arg_size++;
                    }

                    // ! This is to pop the PUSH_NULL
                    value_stack_ptr.pop();

                    auto callable = value_stack_ptr.top();
                    value_stack_ptr.pop();

                    auto fn = std::dynamic_pointer_cast<FunctionPyObject>(std::get<Value::py_object_ptr>(callable.data()));
                    if (!fn) {
                        return Result::RUNTIME_ERROR;
                    }
                    
                    std::flat_map<std::string, Value> local_vars;
                    const auto& func_params = fn->get_params();
                    for (std::size_t i = 0; i < func_params.size(); ++i) {
                        local_vars.emplace(func_params[i], local_values[i]);
                    }
                    
                    const auto idx = fn->get_chunk_index();
                    auto& chunk = m_all_chunks[idx];
                    
                    auto curr_frame = CallFrame {
                        .caller_bp = m_module_bp->byte_offset,
                        .local_vars = local_vars,
                        .chunk = &chunk
                    };
                    m_frames.push(curr_frame);
                    m_curr_frame_ptr = &curr_frame;
                    
                    m_ip++;
                    break;
                }
                case OpCode::POP_JUMP_IF_FALSE: {
                    if (const auto& tos = value_stack_ptr.top(); !tos.is_truthy()) {
                        value_stack_ptr.pop();
                        m_ip = m_module_code[m_ip].argument;
                    } else {
                        value_stack_ptr.pop();
                        m_ip++;
                    }

                    empty_call_frames();
                    break;
                }
                case OpCode::POP_JUMP_IF_TRUE: {
                    if (const auto& tos = value_stack_ptr.top(); tos.is_truthy()) {
                        value_stack_ptr.pop();
                        m_ip = m_module_code[m_ip].argument;
                    } else {
                        value_stack_ptr.pop();
                        m_ip++;
                    }

                    empty_call_frames();
                    break;
                }
                case OpCode::JUMP_BACKWARD: {
                    m_ip = m_module_code[m_ip].argument;
                    // TODO
                    break;
                }
                default: break;
            }
        }
        return Result::OK;
    }
}