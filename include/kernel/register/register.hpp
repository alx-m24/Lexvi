#pragma once

#include <cstdint>
#include <concepts>
#include <tuple>
#include <limits>

namespace kernel {
    // Bit within a register that needs to be writing 1 to be cleared
    class C_Bit {
        private:
            bool m_val{};
            bool m_clear{};

        public:
            C_Bit() = default;
            explicit C_Bit(bool val) : m_val(val) {}
            C_Bit& operator=(bool val) {
                m_val = val;
                m_clear = false;
                return *this;
            }

            inline void clear() {
                m_val = 0;
                m_clear = 1;
            }

            inline bool getValue() {
                return m_val;
            }

            constexpr bool operator()() const {
                return m_clear;
            }
    };

    // Custom constraint that ensures it is an unsigned integer but NOT a boolean
    template<typename T>
    concept RegType = std::unsigned_integral<T> && !std::same_as<T, bool>;

    template<typename T>
    concept FieldType = std::unsigned_integral<T> || std::same_as<T, bool> || std::same_as<T, C_Bit>;

    // struct to Manipulate bit range of Type = T, from register of Type=R. For bits start=Offset to Offset + Width
    template<FieldType T, RegType R, uint8_t StartBit, uint8_t EndBit = StartBit, bool AlignRight = StartBit == EndBit>
    struct Field {
        using TYPE = T;

        static_assert(EndBit >= StartBit);
        static_assert(EndBit < std::numeric_limits<R>::digits);
        
        static constexpr uint8_t Width = (EndBit - StartBit) + 1;
        static constexpr uint8_t Offset = StartBit;

        static_assert(Width != std::numeric_limits<R>::digits);
        static_assert(!AlignRight || std::numeric_limits<T>::digits >= Width);

        static constexpr R MASK = ((R{1} << Width) - R{1}) << Offset;
    
        static constexpr T extract(R raw) {
            if constexpr (AlignRight)
                return static_cast<T>((raw & MASK) >> Offset);
            else
                return static_cast<T>((raw & MASK));
        }
    
        static constexpr R encode(T value) {
            if constexpr (AlignRight)
                return (static_cast<R>(value) << Offset) & MASK;
            else
                return static_cast<R>(value) & MASK;
        }
    };

    template<RegType R, uint8_t StartBit>
    struct Field<C_Bit, R, StartBit, StartBit, true> {
        using TYPE = C_Bit;
    
        static constexpr R MASK = R{1} << StartBit;
    
        static C_Bit extract(R raw) {
            return C_Bit{(raw & MASK) != 0};
        }
    
        static constexpr R encode(const C_Bit& value) {
            return value() ? MASK : R{0};
        }
    };
    
    template<RegType R, typename... Fields>
    struct FieldMask {
        static constexpr R VALUE = (R{0} | ... | Fields::MASK);
    };

    template<RegType T, T INVALID_STATE, typename... Fields>
    class Register {
        private:
            const bool valid{};

        protected:
            T m_raw;
            std::tuple<typename Fields::TYPE...> m_fields{};

        public:
            Register() = default;
            constexpr Register(T val) : valid(val != INVALID_STATE), m_raw(val), m_fields(Fields::extract(val)...) {}

        private:
            template<typename Wanted>
            static consteval std::size_t fieldIndex() {
                std::size_t index = 0;
                std::size_t result = 0;
            
                ((std::is_same_v<Wanted, Fields>
                    ? result = index
                    : ++index), ...);
            
                return result;
            }

            template<std::size_t... I>
            constexpr T encodeFields(
                T raw,
                std::index_sequence<I...>
            ) const {
                return (
                    raw | ...
                    | std::tuple_element_t<I, std::tuple<Fields...>>::encode(
                        std::get<I>(m_fields)
                    )
                );
            }

        protected:
            template<typename Field>
            constexpr typename Field::TYPE& field() {
                return std::get<fieldIndex<Field>()>(m_fields);
            }
            
            template<typename Field>
            constexpr const typename Field::TYPE& field() const {
                return std::get<fieldIndex<Field>()>(m_fields);
            }

        public:
            template<typename Field>
            constexpr void set(typename Field::TYPE value) {
                field<Field>() = value;
            }

            // Commit all field changes into the raw representation and return it
            T operator()() {
                T raw = m_raw & ~FieldMask<T, Fields...>::VALUE;
            
                m_raw = encodeFields(
                    raw,
                    std::index_sequence_for<Fields...>{}
                );
                return m_raw;
            }

            explicit operator bool() const {
                return valid;
            }

            bool operator==(bool) const {
                return static_cast<bool>(*this);
            }
    };

    template<RegType T, T INVALID_STATE, typename... Fields>
    class ReadOnlyRegister : public Register<T, INVALID_STATE, Fields...> {
        public:
            ReadOnlyRegister() = default;
            constexpr ReadOnlyRegister(T val) : Register<T, INVALID_STATE, Fields...>(val) {}
    
            template<typename Field>
            constexpr const typename Field::TYPE& get() const {
                return this->template field<Field>();
            }
            template<typename Field>
            constexpr void set(typename Field::TYPE value) = delete;

            constexpr T operator()() const = delete;
    };

    template<RegType T, T INVALID_STATE, typename... Fields>
    class WriteOnlyRegister : public Register<T, INVALID_STATE, Fields...> {
        public:
            WriteOnlyRegister() = default;
            constexpr WriteOnlyRegister(T val) : Register<T, INVALID_STATE, Fields...>(val) {}
    
            template<typename Field>
            constexpr const typename Field::TYPE& get() const = delete;

            template<typename Field>
            constexpr typename Field::TYPE& get() = delete;
    };

    template<RegType T, T INVALID_STATE, typename... Fields>
    class ReadWriteRegister : public Register<T, INVALID_STATE, Fields...> {
        public:
            ReadWriteRegister() = default;
            constexpr ReadWriteRegister(T val) : Register<T, INVALID_STATE, Fields...>(val) {}

            template<typename Field>
            constexpr const typename Field::TYPE& get() const {
                return this->template field<Field>();
            }

            template<typename Field>
            constexpr typename Field::TYPE& get() {
                return this->template field<Field>();
            }
    };
}
