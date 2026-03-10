#pragma once

#include <stdint.h>

namespace kernel {
    void KeyBoardTick();
    void HandleKeyBoardIRQ();

    namespace keyboard {
        constexpr uint8_t KEY_UP    = 0x80;
        constexpr uint8_t KEY_DOWN  = 0x81;
        constexpr uint8_t KEY_LEFT  = 0x82;
        constexpr uint8_t KEY_RIGHT = 0x83;
    
        bool readEscape(uint8_t& key);
        bool isKeyHeld(unsigned char scanCode);
        char getChar();
    }
}
