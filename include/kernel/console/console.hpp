#pragma once

#include <stdint.h>

namespace kernel {
    struct VGA_Character {
        char character;
        char colorAttribute;

        operator bool() const {
            return character && colorAttribute;
        }
    };
    
    enum class Color : unsigned char {
        BLACK_ON_WHITE = 0xF0,
        WHITE_ON_BLACK = 0x0F,
        RED_ON_BLACK   = 0x04,
        GREEN_ON_BLACK = 0x02,
        BLUE_ON_BLACK  = 0x01,
        YELLOW_ON_BLACK = 0x0E,
        CYAN_ON_BLACK   = 0x0B,
        MAGENTA_ON_BLACK = 0x0D,
        FULL_BLACK = 0x00
    };
    char getColorAttribute(Color colorAttribute);

    void clearConsole();

    void setColor(Color color);

    class ScopedColor {
        private:
            Color previousColor;

        public:
            ScopedColor(const Color& color);
            ~ScopedColor();
    };

    inline void printf() {}
    void printf(char c);
    void printf(const char* str);
    void printf(uint64_t n);
    void printf(uint32_t n);
    void printf(uint16_t n);
    void printf(int n);
    void printfHex(uint64_t n);

    template<typename T, typename... Other>
    void printf(const T& first, const Other&... other) {
        printf(first);
        printf(other...);
    }

    void backSpace();

    enum class MovementDirection : bool {
        LEFT,
        RIGHT,
    };
    void moveVisualCursor(MovementDirection movementDirection);

    void cursorTick();
}
