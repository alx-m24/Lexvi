#pragma once

#include <stdint.h>
#include <type_traits>

#include "kernel/console/console.hpp"
#include "drawable.hpp"

#define printAddress(window, x) \
            do {         \
                window.printf(#x, ": "); \
                window.printfHex(reinterpret_cast<uint64_t>(&x)); \
                window.printf('\n'); \
            } while(false) \
 

namespace kernel {
    class Window {
        private:
            const Window* m_parent = nullptr;

            uint16_t m_width = 80;
            uint16_t m_height = 50;

            // Relative to offset
            uint16_t m_minPrintX = 0;
            uint16_t m_minPrintY = 0;
            uint16_t m_maxPrintX = m_width;
            uint16_t m_maxPrintY = m_height;

            // columns of offset from the left edge
            uint16_t m_offsetX = 0;
            // rows of offset from the top edge
            uint16_t m_offsetY = 0;

            // Relative to offset
            uint16_t m_cursorX = 0;
            uint16_t m_cursorY = 0;

            Color m_activeColor = Color::WHITE_ON_BLACK;
            VGA_Character m_hoveredChar = {};

        public:
            Window();
            Window(const Window* parent, uint16_t width, uint16_t height, uint16_t offsetX, uint16_t offsetY);

        public:
            bool isParent() const {
                return m_parent == nullptr;
            }

        public:
            uint64_t getIndex(uint32_t col, uint32_t row) const;

        public:
            void AdvanceCursor();
            void RetreatCursor();

            void moveVisualCursor(MovementDirection movementDirection);

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

        public:
            void setPrintableArea(uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY);

        public:
            uint16_t getWidth() const { return m_width; }
            uint16_t getHeight() const { return m_height; } 

        public:
            void Draw(const Drawable& drawable) { drawable.Draw(*this); }

            void DrawCharacter(uint16_t x, uint16_t y, char fillChar = 0xDB);
            void DrawCharacter(uint16_t x, uint16_t y, VGA_Character fillChar);

        public:
            void setActiveColor(Color color);

            class ScopedColor {
                private:
                    Window& window;
                    Color m_previousColor;

                public:
                    ScopedColor(Window& window, Color color) : window(window), m_previousColor(window.m_activeColor) {
                        window.m_activeColor = color; 
                    }
                    ~ScopedColor() {
                        window.m_activeColor = m_previousColor;
                    }
            };
            ScopedColor setScopedColor(Color color) { return ScopedColor(*this, color); }

        private: 
            uint64_t lastBlink = 0;
            bool shown = false;
        public:
            void BlinkCursor();

        public:
            static VGA_Character* getVGAMemory() {
                VGA_Character* VGA_MEMORY = reinterpret_cast<VGA_Character*>(0xB8000);
                return VGA_MEMORY;
            }
    };

}
