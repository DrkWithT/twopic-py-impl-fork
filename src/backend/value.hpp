#ifndef TWOPY_VALUE_HPP
#define TWOPY_VALUE_HPP

#include <type_traits>
#include <utility>
#include <variant>

#include "backend/objects.hpp"

namespace TwoPy::Backend {
    enum class ValueTag : uint8_t {
        NONE,   // C++ std::monostate
        BOOL,   // C++ long of 0 or 1
        INT,    // C++ long
        FLOAT,  // C++ double
        REF,    // Value reference
        OBJ,    // Python object
    };

    class Value;

    using Reference = Value*;

    class Value {
    public:
        using py_object_ptr = ObjectBase*;
        using hidden_data = std::variant<std::monostate, long, double, Reference, py_object_ptr>; // make `str` a built-in object type?
        
    private:
        template <typename NativeType>
        struct native_type_tag {
            static constexpr auto value = ValueTag::NONE;
        };

        template <>
        struct native_type_tag <long> {
            static constexpr auto value = ValueTag::INT;
        };

        template <>
        struct native_type_tag <double> {
            static constexpr auto value = ValueTag::FLOAT;
        };

        template <>
        struct native_type_tag <Reference> {
            static constexpr auto value = ValueTag::REF;
        };

        template <>
        struct native_type_tag <py_object_ptr> {
            static constexpr auto value = ValueTag::OBJ;
        };

        hidden_data m_data;
        ValueTag m_tag;

    public:
        constexpr Value() noexcept (std::is_nothrow_default_constructible_v<hidden_data>)
        : m_data {}, m_tag {ValueTag::NONE} {}

        /**
         * @brief Universal constructor for non-None Values in TwoPy. These values need an argument of a C++ type to initialize.
         * 
         * @tparam NativeT Must be accepted by `Value::hidden_data` for construction.
         * @param d 
         */
        template <typename NativeT> requires requires (hidden_data d) {d = NativeT {};}
        constexpr Value(NativeT&& native_value)
        noexcept (std::is_nothrow_constructible_v<hidden_data, NativeT>)
        : m_data (std::forward<NativeT>(native_value)), m_tag {native_type_tag<std::remove_cvref_t<NativeT>>::value} {}

        constexpr bool to_bool(this auto&& self) noexcept {
            switch (self.m_tag) {
            case ValueTag::NONE: return false;
            case ValueTag::BOOL:
            case ValueTag::INT: return std::get<long>(self.m_data) != 0L;
            case ValueTag::FLOAT: return std::get<double>(self.m_data) != 0.0;
            case ValueTag::REF: return std::get<Reference>(self.m_data).to_val != nullptr;
            case ValueTag::OBJ: return std::get<py_object_ptr>(self.m_data) != nullptr; // add abstract is_truthy() method for any ObjectBase behind py_object_ptr
            default: return false;
            }
        }

        constexpr long to_long(this auto&& self) noexcept {
            switch (self.m_tag) {
            case ValueTag::NONE: return 0L;
            case ValueTag::BOOL:
            case ValueTag::INT: return std::get<long>(self.m_data);
            case ValueTag::FLOAT: return static_cast<long>(std::get<double>(self.m_data));
            case ValueTag::REF: return std::get<Reference>(self.m_data) != nullptr;
            case ValueTag::OBJ: return std::get<py_object_ptr>(self.m_data) != nullptr; // add abstract is_truthy() method for any ObjectBase behind py_object_ptr
            default: return 0L;
            }
        }

        constexpr Reference ref() noexcept {
            if (m_tag == ValueTag::REF) return std::get<Reference>(m_data);
            return nullptr;
        }

        constexpr py_object_ptr obj_ref() noexcept {
            if (m_tag == ValueTag::OBJ) return std::get<py_object_ptr>(m_data);
            return nullptr;
        }

        constexpr ValueTag tag(this auto&& self) noexcept {
            return self.is_truthy();
        }

        Value operator+(const Value& other) {
            switch (m_tag) {
            case ValueTag::NONE: return Value {};
            case ValueTag::BOOL:
            case ValueTag::INT: return std::get<long>(m_data) != 0L;
            case ValueTag::FLOAT: return std::get<double>(m_data) != 0.0;
            case ValueTag::REF: return std::get<Reference>(m_data) != nullptr;
            case ValueTag::OBJ: return std::get<py_object_ptr>(m_data) != nullptr; // add abstract is_truthy() method for any ObjectBase behind py_object_ptr
            default: return false;
            }

            return *this;
        }

        Value operator-(const Value& other) {
            switch (m_tag) {
            case ValueTag::NONE: return Value {};
            case ValueTag::BOOL:
            case ValueTag::INT: return std::get<long>(m_data) != 0L;
            case ValueTag::FLOAT: return std::get<double>(m_data) != 0.0;
            case ValueTag::REF: return std::get<Reference>(m_data) != nullptr;
            case ValueTag::OBJ: return std::get<py_object_ptr>(m_data) != nullptr; // add abstract is_truthy() method for any ObjectBase behind py_object_ptr
            default: return false;
            }

            return *this;
        }

        /// TODO: add  %, *, and / overloads with div_int()...
    };
}

#endif