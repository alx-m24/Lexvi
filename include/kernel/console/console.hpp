#pragma once

#include <stdint.h>
#include <type_traits>

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
    void printf(double d);
    void printfHex(uint64_t n);

    void printSigned(int64_t n);
    void printUnsigned(uint64_t n);

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

    void backSpace();

    enum class MovementDirection : bool {
        LEFT,
        RIGHT,
    };
    void moveVisualCursor(MovementDirection movementDirection);

    void cursorTick();

    class Window;
    void setLogWindow(Window* window);
}
