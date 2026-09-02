#ifndef TWOPY_OBJECTS_HPP
#define TWOPY_OBJECTS_HPP

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <type_traits>

#include <format>

/* Each of these would be local bytecode scope */
namespace TwoPy::Backend {
    //class Value;

    /* These would be polymorphic heap object and wouldn't be primitives such as floats, bools, and ints */

    /* PyObjects get compiled from the PVM and it detects whether its a default value or heap based */    

    enum class ObjectTag : uint8_t {
        NONE,       // non-object
        LIST,
        // DICT,
        // CLASS,
        FUNCTION,   // callable object
        NATIVE,     // built-in callable: no chunk, the VM runs it inline
        STRING,
    };

    /* Built-ins are dispatched on this instead of by name-compare at call time. */
    enum class NativeId : std::uint8_t {
        PRINT,
        RANGE
    };

    /* Insprided by Derkt's ObjectBase class which allows for Polymophric virutal representation */
    struct ObjectBase {
        virtual ~ObjectBase() = default;

        [[nodiscard]] virtual ObjectTag tag() const noexcept = 0;

        /* Indexing */
        /// NOTE: immutable accessor for impl. of __get__
        // virtual const Value& operator[](const Value&) const = 0;

        /// NOTE: The `vm_state` argument must hide a `VMContext*` to affect the stack of. 
        // This `void*` trick & reinterpret_cast is needed to dodge circular inclusions- What if the VM uses the Value & object types but those must know of the VM internals? @DrkWithT
        /// NOTE: for __call__(self, args)
        /* I'll implament this when I start on my vm */
        //virtual bool call([[maybe_unused]] void* vm_state, [[maybe_unused]] uint8_t arg_count) = 0;

       /// NOTE: for __str__(self) converts different types to string
       /// Useful for printing out debug stmts inside the bytecode print
        virtual std::string stringify() = 0;

        [[nodiscard]] virtual bool is_truthy() const noexcept = 0;
    };

    class FunctionPyObject : public ObjectBase {
        private:
            std::string m_name;
            std::vector<std::string> m_params;
            std::uint8_t m_chunk_index {};

        public:
            explicit FunctionPyObject(std::string name, std::vector<std::string> params, std::uint8_t chunk_index)
                : m_name(std::move(name)), m_params(std::move(params)), m_chunk_index(chunk_index) {}

            [[nodiscard]] ObjectTag tag() const noexcept override {
                return ObjectTag::FUNCTION;
            }

            [[nodiscard]] const std::string& name() const noexcept {
                return m_name;
            }

            /* Useful for printing out debug */
            [[nodiscard]] std::uint8_t get_chunk_index() const noexcept {
                return m_chunk_index;
            }

            /* Useful for printing out debug */
            [[nodiscard]] const std::vector<std::string>& get_params() const noexcept {
                return m_params;
            }

            /* Gets called when the printer calls it */
            [[nodiscard]] std::string stringify() override {
                return std::format("<code object {} at {}>", m_name, static_cast<const void*>(this));
            }

            [[nodiscard]] bool is_truthy() const noexcept override {
                return !m_name.empty();
            }
    };

    class NativePyObject : public ObjectBase {
        private:
            std::string m_name;
            NativeId m_id;

        public:
            explicit NativePyObject(std::string name, NativeId id)
                : m_name(std::move(name)), m_id(id) {}

            [[nodiscard]] ObjectTag tag() const noexcept override {
                return ObjectTag::NATIVE;
            }

            [[nodiscard]] const std::string& name() const noexcept {
                return m_name;
            }

            [[nodiscard]] NativeId id() const noexcept {
                return m_id;
            }

            [[nodiscard]] std::string stringify() override {
                return std::format("<built-in function {}>", m_name);
            }

            [[nodiscard]] bool is_truthy() const noexcept override {
                return true;
            }
    };

    class StringPyObject : public ObjectBase {
        private:
            std::string m_data {};

        public:
            explicit StringPyObject(std::string data) : m_data(std::move(data)) {}

            [[nodiscard]] ObjectTag tag() const noexcept override {
                return ObjectTag::STRING;
            }

            // const Value& operator[](const Value&) const override {
                
            // }

            /* [[nodiscard]] bool call([[maybe_unused]] void* vm_state, [[maybe_unused]] uint8_t arg_count) override {
                return false;
            }*/

            /* Gets called when the printer calls it */
            std::string stringify() override {
                return m_data;
            }

            // Empty strings are falsy, non-empty strings are truthy (Python behavior)
            [[nodiscard]] bool is_truthy() const noexcept override {
                return !m_data.empty();
            }
    };
}

#endif
