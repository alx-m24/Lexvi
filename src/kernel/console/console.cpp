#include "kernel/console/console.hpp"
#include "kernel/console/tui/window.hpp"
#include "kernel/time/time.hpp"

namespace kernel {
   VGA_Character* VGA_MEMORY = reinterpret_cast<VGA_Character*>(0xB8000);
    
    int cursorCol = 0;
    int cursorRow = 0;

    uint64_t lastBlink = 0;
    bool shown = false;
    Color currentColorAttribute = Color::WHITE_ON_BLACK;

    Window* logWindow = nullptr;
    
    constexpr unsigned int MAX_COLUMN = 80;
    constexpr unsigned int MAX_ROWS = 25;
    
    constexpr static unsigned int VGA_INDEX(const unsigned int col, const unsigned int row) {
        return col + row * MAX_COLUMN;
    }

    char getColorAttribute(Color colorAttribute) {
        return static_cast<char>(colorAttribute);
    }

    static void AdvanceCursor() {
        ++cursorCol;
        if (cursorRow >= MAX_ROWS) { cursorRow = MAX_ROWS - 1; }
        if (cursorCol >= MAX_COLUMN) { cursorCol = 0; ++cursorRow; }
    }

    static void MoveBackCursor() {
        if (cursorCol == 0) {
            if (cursorRow == 0) return;
            cursorCol = MAX_COLUMN - 1;
            cursorRow -= 1;
        }
        else {
            cursorCol -= 1;
        }
    }
    
    void clearConsole() {
        if (logWindow != nullptr) { logWindow->clear(); return; }
        for (int i = 0; i < MAX_COLUMN * MAX_ROWS; ++i) {
            VGA_MEMORY[i] = { .character = '\0', .colorAttribute = getColorAttribute(Color::FULL_BLACK) };
        }
        cursorCol = cursorRow = 0;
    }

    void setColor(Color color) {
        if (logWindow != nullptr) { logWindow->setActiveColor(color); return; }
        currentColorAttribute = color;
    }

    ScopedColor::ScopedColor(const Color& color) {
        this->previousColor = currentColorAttribute;
        currentColorAttribute = color;
        if (logWindow != nullptr) logWindow->setActiveColor(color);
    }
    ScopedColor::~ScopedColor() {
        currentColorAttribute = this->previousColor;
        if (logWindow != nullptr) logWindow->setActiveColor(this->previousColor);
    }

    // forward declaration
    void cursorTick();

    void printf(char c) {
        if (logWindow != nullptr) { logWindow->printf(c); return; }
        if (c == '\0') return;
        if (c == '\b') { backSpace(); return; }
        if (c == '\t') {
            int nextTab = (cursorCol + 4) & ~3;
            while (cursorCol < nextTab) printf(' ');
            return;
        }
        if (c == '\n') {
            VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { };
            ++cursorRow;
            if (cursorRow >= MAX_ROWS) cursorRow = MAX_ROWS - 1;
            cursorCol = 0;
            return;
        }
        if (c == '\r') {
            cursorCol = 0;
            return;
        }
        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { .character = c, .colorAttribute = getColorAttribute(currentColorAttribute) };
        AdvanceCursor();
    }

    void printf(const char* str) {
        // if (logWindow != nullptr) { logWindow->printf(str); return; }
        for (; *str != '\0'; ++str) printf(*str);
    }
   
    void printf(uint32_t n) {
        if (logWindow != nullptr) { logWindow->printf(n); return; }
        if (n == 0) { printf('0'); return; }
        char buffer[10];
        int i = 0;
        while (n > 0) { buffer[i++] = (n % 10) + '0'; n = n / 10; }
        while (i > 0) { printf(buffer[--i]); }
    }
    
    void printf(uint64_t n) {
        if (logWindow != nullptr) { logWindow->printf(n); return; }
        if (n == 0) { printf('0'); return; }
        char buffer[20];
        int i = 0;
        while (n > 0) { buffer[i++] = (n % 10) + '0'; n = n / 10; }
        while (i > 0) { printf(buffer[--i]); }
    }

    void printf(double d) {
        if (logWindow != nullptr) { logWindow->printf(d); return; }
        if (d != d) { printf("NaN"); return; }          // NaN check
        if (d < -1e308) { printf("-Inf"); return; }      // -Infinity
        if (d > 1e308)  { printf("Inf");  return; }      // +Infinity
        if (d > 1e19) { printf("(too large)"); return; } // avoiding numbers larger than UINT64_MAX
    
        if (d < 0) {
            printf('-');
            d = -d;
        }
    
        // print integer part
        printf(static_cast<uint64_t>(d));
        printf('.');
    
        // print fractional part to 6 decimal places
        d -= static_cast<uint64_t>(d);
        for (int i = 0; i < 6; ++i) {
            d *= 10;
            printf(static_cast<char>('0' + static_cast<uint64_t>(d) % 10));
        }
    }

    void printf(uint16_t n) {
        if (logWindow != nullptr) { logWindow->printf(n); return; }
        printf(static_cast<uint32_t>(n));
    }
    
    void printf(int n) {
        if (logWindow != nullptr) { logWindow->printf(n); return; }
        if (n < 0) { printf('-'); n = -n; }
        printf(static_cast<unsigned int>(n));
    }

    void printfHex(uint64_t n) {
        if (logWindow != nullptr) { logWindow->printfHex(n); return; }
        char buffer[16];
        for(int i = 15; i >= 0; --i) {
            uint64_t digit = (n >> (i*4)) & 0xF;
            buffer[15-i] = digit < 10 ? '0'+digit : 'A'+(digit-10);
        }
        printf('0');
        printf('x');
        for(int i = 0; i < 16; ++i) printf(buffer[i]);
    }

    void backSpace() {
        if (logWindow != nullptr) { logWindow->printf('\b'); return; }
        // clear current cursor cell, retreat, clear again (same as Window)
        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = {};
        MoveBackCursor();
        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = {};
        // reset blink so cursor appears immediately
        lastBlink = 0;
        cursorTick();
    }

    VGA_Character hoveredChar = {};
    void moveVisualCursor(MovementDirection movementDirection) {
        if (logWindow != nullptr) { logWindow->moveVisualCursor(movementDirection); return; }

        // restore character under cursor before moving
        VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = hoveredChar;

        if (movementDirection == MovementDirection::LEFT)
            MoveBackCursor();
        else
            AdvanceCursor();

        // save and blank new cursor position, reset blink
        hoveredChar = VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)];
        lastBlink = 0;
        shown = false;
        cursorTick();
    }


    void cursorTick() {
        if (logWindow != nullptr) { logWindow->BlinkCursor(); return; }
        constexpr uint64_t BLINK_FREQ = 4;
        constexpr uint64_t BLINK_INTERVAL = CLOCK_FREQ / BLINK_FREQ;
        uint64_t now = getCurrentTick();
        if (now - lastBlink < BLINK_INTERVAL) return;
        lastBlink = now;

        shown = !shown;
        if (shown) {
            hoveredChar = VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)];
            VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = { '_', getColorAttribute(currentColorAttribute) };
        } else {
            VGA_MEMORY[VGA_INDEX(cursorCol, cursorRow)] = hoveredChar;
        }
    }

    void setLogWindow(Window* window) { logWindow = window; }
}
