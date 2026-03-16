#pragma once

#include <stdint.h>
#include <type_traits>

#include "kernel/console/console.hpp"

namespace kernel {
    class Window {
        private:
            const Window* m_parent = nullptr;

            uint16_t m_width = 80;
            uint16_t m_height = 50;

            // columns of offset from the left edge
            uint16_t m_offsetX = 0;
            // rows of offset from the top edge
            uint16_t m_offsetY = 0;

            // Relative to offset
            uint16_t m_cursorX = 0;
            uint16_t m_cursorY = 0;

            Color m_activeColor = Color::WHITE_ON_BLACK;

        public:
            Window();
            Window(const Window* parent, uint16_t width, uint16_t height, uint16_t offsetX, uint16_t offsetY);

        public:
            bool isParent() const {
                return m_parent == nullptr;
            }

        public:
            uint64_t getIndex(uint32_t col, uint32_t row) const;

        private:
            void AdvanceCursor();
            void RetreatCursor();

        private:
            void printf() const {}

        public:
            void clear();

            void printf(char c);
            void printf(const char* str);
            void printf(double d);
            void printfHex(uint64_t n);

            template<typename T>
            requires (std::is_integral_v<T> && !std::is_same_v<T, char> && !std::is_pointer_v<T>)
            void printf(T n) {
                if constexpr (std::is_signed_v<T>) {
                    printSigned(static_cast<int64_t>(n));
                }
                else {
                    printUnsigned(static_cast<uint64_t>(n));
                }
            }

            template<typename First, typename... Others>
            void printf(const First& f, const Others&... others) {
                printf(f);
                printf(others...);
            }
            
        private:
            void printSigned(int64_t n);
            void printUnsigned(uint64_t n);
    };
}
