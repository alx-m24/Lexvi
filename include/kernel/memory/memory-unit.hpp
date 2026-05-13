#pragma once

// While this unit is within kernel include
// It can be used outside of it
// It was greatly inspired by std::chrono & it's timepoints

#include <cstdint>
#include <type_traits>

template<typename T>
concept ByteRep_T = std::is_floating_point_v<T> || std::is_integral_v<T>;

using ByteMultiple_T = uint64_t;

// --- Forward Declaration ---
template<ByteRep_T Rep, ByteMultiple_T Scale>
class MemorySize;

using Bytes = MemorySize<uint64_t, 1>;
using KiB   = MemorySize<uint64_t, 1024>;
using MiB   = MemorySize<uint64_t, 1024 * 1024>;
using GiB   = MemorySize<uint64_t, 1024ULL * 1024 * 1024>;
using TiB   = MemorySize<uint64_t, 1024ULL * 1024 * 1024 * 1024>;

template<typename To, typename Rep, ByteMultiple_T Scale>
constexpr To memory_cast(MemorySize<Rep, Scale> from) {
    return To(static_cast<To::Representation>(*from) / *To(1));
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
            auto b = **this;
            auto a = *alignment;
            return This_T(((b + a - 1) / a) * a);
        }

    public:
        constexpr uint64_t operator*() const {
            return bytes().count();
        }

        template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
        constexpr bool operator==(const MemorySize<OtherRep, OtherScale>& other) const {
            return **this == *other;
        }

        template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
        constexpr This_T operator+(const MemorySize<OtherRep, OtherScale>& other) const {
            return MemorySize(this->count() + memory_cast<This_T>(other).count());
        }

        template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
        constexpr This_T operator-(const MemorySize<OtherRep, OtherScale>& other) const {
            return MemorySize(this->count() - memory_cast<This_T>(other).count());
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
            return **this < *other;
        }

        template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
        constexpr bool operator>(const MemorySize<OtherRep, OtherScale>& other) const {
            return **this > *other;
        }

        template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
        constexpr bool operator<=(const MemorySize<OtherRep, OtherScale>& other) const {
            return **this <= *other;
        }

        template<ByteRep_T OtherRep, ByteMultiple_T OtherScale>
        constexpr bool operator>=(const MemorySize<OtherRep, OtherScale>& other) const {
            return **this >= *other;
        }
};

// Bytes
constexpr Bytes operator""_B(unsigned long long v) {
    return Bytes(v);
}

constexpr Bytes operator""_B(long double v) {
    return Bytes(static_cast<uint64_t>(v));
}

// KiB
constexpr KiB operator""_KiB(unsigned long long v) {
    return KiB(v);
}

constexpr MemorySize<uint64_t,1> operator""_KiB(long double v) {
    return MemorySize<uint64_t,1>(static_cast<uint64_t>(v * 1024.0));
}

// MiB
constexpr MiB operator""_MiB(unsigned long long v) {
    return MiB(v);
}

constexpr MemorySize<uint64_t,1> operator""_MiB(long double v) {
    return MemorySize<uint64_t,1>(static_cast<uint64_t>(v * 1024.0 * 1024.0));
}

// GiB
constexpr GiB operator""_GiB(unsigned long long v) {
    return GiB(v);
}

constexpr MemorySize<uint64_t,1> operator""_GiB(long double v) {
    return MemorySize<uint64_t,1>(static_cast<uint64_t>(v * 1024.0 * 1024.0 * 1024.0));
}

// TiB
constexpr TiB operator""_TiB(unsigned long long v) {
    return TiB(v);
}

constexpr MemorySize<uint64_t,1> operator""_TiB(long double v) {
    return MemorySize<uint64_t,1>(static_cast<uint64_t>(v * 1024.0 * 1024.0 * 1024.0 * 1024.0));
}
