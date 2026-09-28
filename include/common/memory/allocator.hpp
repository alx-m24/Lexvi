// include/common/memory/allocator.hpp

#pragma once

#include <cstddef>
#include <limits>

#include "common/error/assert.hpp"

namespace Lexvi::Memory {

    // To be implemented in .cpp file (boot or kernel specific)
    void* Allocate(std::size_t bytes, std::size_t alignment);
    void Deallocate(void* ptr, std::size_t bytes, std::size_t alignment) noexcept;
    
    
    template<typename T>
    struct Allocator {
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
    
        template<typename U>
        struct rebind {
            using other = Allocator<U>;
        };
    
        constexpr Allocator() noexcept = default;
    
        template<typename U>
        constexpr Allocator(const Allocator<U>&) noexcept {}
    
        [[nodiscard]]
        T* allocate(size_type count) {
            LEXVI_ASSERT(count <= max_size());
    
            return static_cast<T*>(
                Allocate(
                    count * sizeof(T),
                    alignof(T)
                )
            );
        }
    
        void deallocate(T* ptr, size_type count) noexcept {
            Deallocate(
                ptr,
                count * sizeof(T),
                alignof(T)
            );
        }
    
        static constexpr size_type max_size() noexcept {
            return std::numeric_limits<size_type>::max() / sizeof(T);
        }
    };
}
