#pragma once

namespace kernel {
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
    void printf(unsigned int n);
    void printf(int n);
    void printfHex(unsigned int n);

    template<typename T, typename... Other>
    void printf(const T& first, const Other&... other) {
        printf(first);
        printf(other...);
    }
}
