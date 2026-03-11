#include "kernel/keyboard/keyboard.hpp"

#include "kernel/time/time.hpp"
#include "asm/instructions.hpp"

#include <stdint.h>

namespace kernel {
    namespace {
        constexpr char scancode_to_ascii[] = {
        //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
            0,    0,   '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t', // 0x00
           'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n',  0,   'a',  's',  // 0x10
           'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',   0,  '\\', 'z',  'x',  'c',  'v',  // 0x20
           'b',  'n',  'm',  ',',  '.',  '/',   0,   '*',   0,   ' ',   0                                   // 0x30
        };
        constexpr char shift_scancode_to_ascii[] = {
        //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
            0,    0,   '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t', // 0x00
           'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}',  '\n',  0,   'A',  'S',  // 0x10
           'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"',  '~',   0,   '|',  'Z',  'X',  'C',  'V',  // 0x20
           'B',  'N',  'M',  '<',  '>',  '?',   0,   '*',   0,   ' ',   0                                   // 0x30
        };
        constexpr char caps_scancode_to_ascii[] = {
        //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
            0,    0,   '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t', // 0x00
           'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '[',  ']',  '\n',  0,   'A',  'S',  // 0x10
           'D',  'F',  'G',  'H',  'J',  'K',  'L',  ';', '\'',  '`',   0,  '\\', 'Z',  'X',  'C',  'V',  // 0x20
           'B',  'N',  'M',  ',',  '.',  '/',   0,   '*',   0,   ' ',   0                                   // 0x30
        };

        // --- state ---
        bool keys_held[256] = {};
        uint64_t last_press_time[256] = {};
        uint64_t next_repeat_time[256] = {};

        constexpr uint64_t REPEAT_DELAY = 500; // ms
        constexpr uint64_t REPEAT_RATE  = 33;  // ms

        bool shift_held = false;
        bool caps_lock  = false;

        bool extended = false;

        // --- event queue ---
        constexpr uint32_t KEY_QUEUE_SIZE = 64;
        char key_queue[KEY_QUEUE_SIZE] = {};
        uint32_t queue_head = 0;
        uint32_t queue_tail = 0;

        void enqueue(char c) {
            uint32_t next = (queue_tail + 1) % KEY_QUEUE_SIZE;
            if (next == queue_head) return; // full, drop
            key_queue[queue_tail] = c;
            queue_tail = next;
        }

        char dequeue() {
            if (queue_head == queue_tail) return '\0'; // empty
            char c = key_queue[queue_head];
            queue_head = (queue_head + 1) % KEY_QUEUE_SIZE;
            return c;
        }

        char resolveChar(unsigned char scanCode) {
            if (scanCode >= sizeof(scancode_to_ascii)) return 0;
            if (shift_held) return shift_scancode_to_ascii[scanCode];
            if (caps_lock)  return caps_scancode_to_ascii[scanCode];
            return scancode_to_ascii[scanCode];
        }
    }

    // --- public API ---
    namespace keyboard {
        bool readEscape(uint8_t& key) {
            if (keyboard::getChar() != '[') return false;
            char dir = keyboard::getChar();
            switch (dir) {
                case 'A': key = KEY_UP;    return true;
                case 'B': key = KEY_DOWN;  return true;
                case 'C': key = KEY_RIGHT; return true;
                case 'D': key = KEY_LEFT;  return true;
            }
            return false;
        }

        bool isKeyHeld(unsigned char scanCode) {
            return keys_held[scanCode & 0x7F];
        }

        char getChar() {
            return dequeue();
        }
    }

    void KeyBoardTick() {
        uint64_t now = getCurrentTick();
        for (int i = 0; i < 256; i++) {
            if (!keys_held[i]) continue;
            if (now >= next_repeat_time[i]) {
                char c = resolveChar(i);
                if (c) enqueue(c);
                next_repeat_time[i] = now + REPEAT_RATE * (CLOCK_FREQ / 1000u);
            }
        }
    }

    void HandleKeyBoardIRQ() {
        unsigned char status = inb(0x64);
        if (!(status & 0b00000001)) return;
        if (status & 0b00100000) return;

        unsigned char scanCode = inb(0x60);

        constexpr unsigned char PRESS_MAX    = 0x7F;
        constexpr unsigned char RELEASE_MIN  = 0x80;

        if (scanCode == 0xE0) {
            extended = true;  // just set the flag, return
            return;
        }
        
        if (extended) {
            extended = false;
            if (scanCode < PRESS_MAX) {
                switch (scanCode) {
                    case 0x48: keys_held[keyboard::KEY_UP]    = true; enqueue('\x1B'); enqueue('['); enqueue('A'); return;
                    case 0x50: keys_held[keyboard::KEY_DOWN]  = true; enqueue('\x1B'); enqueue('['); enqueue('B'); return;
                    case 0x4D: keys_held[keyboard::KEY_RIGHT] = true; enqueue('\x1B'); enqueue('['); enqueue('C'); return;
                    case 0x4B: keys_held[keyboard::KEY_LEFT]  = true; enqueue('\x1B'); enqueue('['); enqueue('D'); return;
                }
            }
            if (scanCode >= RELEASE_MIN) {
                switch (scanCode & 0x7F) {
                    case 0x48: keys_held[keyboard::KEY_UP]    = false; return;
                    case 0x50: keys_held[keyboard::KEY_DOWN]  = false; return;
                    case 0x4D: keys_held[keyboard::KEY_RIGHT] = false; return;
                    case 0x4B: keys_held[keyboard::KEY_LEFT]  = false; return;
                }
            }
            return;
        }

        if (scanCode < PRESS_MAX) {
            // modifier tracking
            if (scanCode == 0x2A || scanCode == 0x36) { shift_held = true;  return; }
            if (scanCode == 0x3A)                     { caps_lock = !caps_lock; return; }

            uint64_t now = getCurrentTick();
            if (now - last_press_time[scanCode] > REPEAT_DELAY) {
                // genuine new press
                keys_held[scanCode] = true;
                next_repeat_time[scanCode] = now + REPEAT_DELAY * (CLOCK_FREQ / 1000u);
                char c = resolveChar(scanCode);
                if (c) enqueue(c);
            }
            last_press_time[scanCode] = now;

        } else if (scanCode >= RELEASE_MIN) {
            unsigned char pressCode = scanCode & 0x7F;
            if (pressCode == 0x2A || pressCode == 0x36) { shift_held = false; return; }
            keys_held[pressCode] = false;
            last_press_time[pressCode] = 0;
        }
    }
}
