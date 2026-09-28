#pragma once

// While this unit is within kernel include
// It can be used outside of it
// It was greatly inspired by std::chrono & it's timepoints

#include <cstdint>
#include <type_traits>

template<typename T>
concept ByteRep_T = std::is_floating_point_v<T> || std::is_integral_v<T>;

namespace Lexvi::Memory {
using ByteMultiple_T = uint64_t;

// --- Forward Declaration ---
template<ByteRep_T Rep, ByteMultiple_T Scale>
class MemorySize;

using Bytes = MemorySize<uint64_t, 1>;
using KiB   = MemorySize<uint64_t, 1024>;
using MiB   = MemorySize<uint64_t, 1024 * 1024>;
using GiB   = MemorySize<uint64_t, 1024UL * 1024 * 1024>;
using TiB   = MemorySize<uint64_t, 1024UL * 1024 * 1024 * 1024>;

template<ByteRep_T ToRep, ByteMultiple_T ToScale, ByteRep_T FromRep, ByteMultiple_T FromScale>
constexpr MemorySize<ToRep, ToScale> memory_cast(MemorySize<FromRep, FromScale> from) {
    return MemorySize<ToRep, ToScale>(static_cast<ToRep>(
            static_cast<long double>(from.count()) * FromScale / ToScale
        )
    );
}

template<ByteRep_T Rep, ByteMultiple_T Scale>
    class MemorySize {
        public:
            using Representation = Rep;
        private:
            Rep value {};
            using This_T = MemorySize<Rep, Scale>;
    
        public:
            MemorySize() = default;

            constexpr explicit MemorySize(Rep val) : value(val) {}

            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr MemorySize(const MemorySize<OtherRep, OtherScale>& other)
            : value(memory_cast<Rep, Scale>(other).count()) {}

            constexpr This_T& operator=(Rep val) {
                this->value = val;
                return *this;
            }
    
        public:
            constexpr Rep count() const {
                return value;
            }
    
            constexpr Bytes bytes() const {
                return Bytes(static_cast<uint64_t>(value) * Scale);
            }
    
        public:
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr This_T align_up(const MemorySize<OtherRep, OtherScale>& alignment) const {
                const uint64_t bytes_value = bytes().count();
                const uint64_t alignment_bytes = alignment.bytes().count();
            
                const uint64_t aligned =
                    ((bytes_value + alignment_bytes - 1) / alignment_bytes)
                    * alignment_bytes;
            
                return memory_cast<Rep, Scale>(Bytes(aligned));
            }
    
        public:
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr bool operator==(const MemorySize<OtherRep, OtherScale>& other) const {
                return bytes().count() == other.bytes().count();
            }
    
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr This_T operator+(const MemorySize<OtherRep, OtherScale>& other) const {
                return MemorySize(this->count() + memory_cast<Rep, Scale>(other).count());
            }
    
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr This_T operator-(const MemorySize<OtherRep, OtherScale>& other) const {
                return MemorySize(this->count() - memory_cast<Rep, Scale>(other).count());
            }
    
            template<ByteRep_T T>
            constexpr This_T operator*(T scalar) const {
                return MemorySize(this->count() * scalar);
            }
    
            template<ByteRep_T T>
            constexpr This_T operator/(T scalar) const {
                return MemorySize(this->count() / scalar);
            }
    
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr Rep operator/(const MemorySize<OtherRep, OtherScale>& other) const {
                return this->bytes().count() / other.bytes().count();
            }
    
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr bool operator<(const MemorySize<OtherRep, OtherScale>& other) const {
                return bytes().count() < other.bytes().count();
            }
    
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr bool operator>(const MemorySize<OtherRep, OtherScale>& other) const {
                return bytes().count() > other.bytes().count();
            }
    
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr bool operator<=(const MemorySize<OtherRep, OtherScale>& other) const {
                return bytes().count() <= other.bytes().count();
            }
    
            template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
            constexpr bool operator>=(const MemorySize<OtherRep, OtherScale>& other) const {
                return bytes().count() >= other.bytes().count();
            }
    };
}

constexpr Lexvi::Memory::KiB operator""_KiB(unsigned long long v) {
    return Lexvi::Memory::KiB(v);
}

constexpr Lexvi::Memory::MiB operator""_MiB(unsigned long long v) {
    return Lexvi::Memory::MiB(v);
}

constexpr Lexvi::Memory::GiB operator""_GiB(unsigned long long v) {
    return Lexvi::Memory::GiB(v);
}

constexpr Lexvi::Memory::TiB operator""_TiB(unsigned long long v) {
    return Lexvi::Memory::TiB(v);
}
