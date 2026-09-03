#include "kernel/debug/gop.hpp"

#include "kernel/kernel-config.hpp"
#include "kernel/memory/memory-defs.hpp"
#include "kernel/fonts/font8x16.h"

#include "kernel/error/error.hpp"

#define VALIDATE_GOP() \
    KERNEL_ASSERT(gop.Info || gop.FrameBufferBase != 0); \
    KERNEL_ASSERT(gop.Info->PixelFormat != GOP_PixelFormat::PixelBltOnly); 

namespace kernel {
    GOP gop{};

    namespace {
        // Simple HSV -> RGB, h in [0,360), s/v in [0,1]. Returns 0-255 components.
        void hsv_to_rgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
            float c = v * s;
            float hDiv60 = h / 60.0f;
            // Manual fmod(hDiv60, 2.0f) since freestanding has no libm.
            float hMod2 = hDiv60 - 2.0f * static_cast<float>(static_cast<int>(hDiv60 / 2.0f));
            float x = c * (1.0f - __builtin_fabsf(hMod2 - 1.0f));
            float m = v - c;
        
            float rf, gf, bf;
            if      (h < 60)  { rf = c; gf = x; bf = 0; }
            else if (h < 120) { rf = x; gf = c; bf = 0; }
            else if (h < 180) { rf = 0; gf = c; bf = x; }
            else if (h < 240) { rf = 0; gf = x; bf = c; }
            else if (h < 300) { rf = x; gf = 0; bf = c; }
            else              { rf = c; gf = 0; bf = x; }
        
            r = static_cast<uint8_t>((rf + m) * 255.0f);
            g = static_cast<uint8_t>((gf + m) * 255.0f);
            b = static_cast<uint8_t>((bf + m) * 255.0f);
        }

        constexpr uint8_t GLYPH_COL_COUNT = 8;
        constexpr uint8_t GLYPH_ROW_COUNT = 16;

        uint32_t CELL_NUM_X{};
        uint32_t CELL_NUM_Y{};
        uint32_t cursor_cell_X = 0;
        uint32_t cursor_cell_Y = 0;
    }

    void load_GOP() {
        gop = *reinterpret_cast<GOP*>(TO_VIRT(GOP_PHYS_ADDRESS));
        gop.Info = reinterpret_cast<GOP_Info*>(TO_VIRT(gop.Info));

        CELL_NUM_X = gop.Info->HorizontalResolution / GLYPH_COL_COUNT;
        CELL_NUM_Y = gop.Info->VerticalResolution / GLYPH_ROW_COUNT;
    }

    void init_FrameBuffer() {
        gop.FrameBufferBase = MMIO_TO_VIRT(gop.FrameBufferBase);
    }    

    constexpr uint32_t GOP::Color::operator()(const GOP_Info& info) const {
        switch (info.PixelFormat) {
            case GOP_PixelFormat::PixelRedGreenBlueReserved8BitPerColor:
                return (static_cast<uint32_t>(r))       |
                       (static_cast<uint32_t>(g) << 8)  |
                       (static_cast<uint32_t>(b) << 16);
    
            case GOP_PixelFormat::PixelBlueGreenRedReserved8BitPerColor:
                return (static_cast<uint32_t>(b))       |
                       (static_cast<uint32_t>(g) << 8)  |
                       (static_cast<uint32_t>(r) << 16);
    
            case GOP_PixelFormat::PixelBitMask: {
                uint32_t redMask   = info.PixelInformation[0];
                uint32_t greenMask = info.PixelInformation[1];
                uint32_t blueMask  = info.PixelInformation[2];
    
                auto scale_into_mask = [](uint32_t mask, uint8_t value) -> uint32_t {
                    if (mask == 0) return 0;
    
                    // Manual trailing-zero-count and popcount, no libgcc builtins.
                    uint32_t shift = 0;
                    while (((mask >> shift) & 1u) == 0) {
                        ++shift;
                    }
    
                    uint32_t width = 0;
                    for (uint32_t m = mask; m != 0; m >>= 1) {
                        width += (m & 1u);
                    }
    
                    uint32_t maxVal = (1u << width) - 1u;
                    uint32_t scaled = (static_cast<uint32_t>(value) * maxVal) / 255u;
                    return scaled << shift;
                };
    
                return scale_into_mask(redMask, r) |
                       scale_into_mask(greenMask, g) |
                       scale_into_mask(blueMask, b);
            }
    
            case GOP_PixelFormat::PixelBltOnly:
            default:
                return 0;
        }
    }
    
    void gop_test() {
        VALIDATE_GOP();
    
        const uint32_t width  = gop.Info->HorizontalResolution;
        const uint32_t height = gop.Info->VerticalResolution;
        const uint32_t pitch  = gop.Info->PixelsPerScanLine;
    
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                // Hue sweeps 0-360 across the width, like a UV.x gradient in a fullscreen shader.
                float hue = (static_cast<float>(x) / static_cast<float>(width)) * 360.0f;
    
                uint8_t r, g, b;
                hsv_to_rgb(hue, 1.0f, 1.0f, r, g, b);
    
                drawPixel(x, y, GOP::Color(r, g, b));
            }
        }
    }

    void gop_fill(GOP::Color color) {
        VALIDATE_GOP();
        gop_fill(0, 0, gop.Info->HorizontalResolution, gop.Info->VerticalResolution, color);
    }

    void gop_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, GOP::Color color) {
        VALIDATE_GOP();

        KERNEL_ASSERT(x < gop.Info->HorizontalResolution);
        KERNEL_ASSERT(y < gop.Info->VerticalResolution);

        uint32_t* fb = reinterpret_cast<uint32_t*>(gop.FrameBufferBase);
        for (uint32_t y_ = y; y_ < gop.Info->VerticalResolution && y_ < y + h; ++y_) {
            for (uint32_t x_ = x; x_ < gop.Info->HorizontalResolution && x_ < x + w; ++x_) {
                drawPixel(x_, y_, color);
            }
        }
    }

    void drawPixel(uint32_t x, uint32_t y, GOP::Color color) {
        VALIDATE_GOP();

        KERNEL_ASSERT(x < gop.Info->HorizontalResolution);
        KERNEL_ASSERT(y < gop.Info->VerticalResolution);

        uint32_t* fb = reinterpret_cast<uint32_t*>(gop.FrameBufferBase);
        uint32_t* row = fb + y * gop.Info->PixelsPerScanLine;
        row[x] = color(*gop.Info);
    }

    void displayGlyph(uint32_t x, uint32_t y, char c, GOP::Color color, GOP::Color background, bool overWriteBackground) {
        VALIDATE_GOP();

        for (uint8_t row = 0; row < GLYPH_ROW_COUNT; ++row) {
            for (uint8_t col = 0; col < GLYPH_COL_COUNT; ++col) {
                bool set = (font8x16[c][row] >> (GLYPH_COL_COUNT - col)) & 1;
                if (set) {
                    drawPixel(x + col, y + row, color);
                }
                else if (overWriteBackground) {
                    drawPixel(x + col, y + row, background);
                }
            }
        }
    }

    void GOP::reset() {
        gop_fill(Color(0, 0, 0));
        cursor_cell_X = cursor_cell_Y = 0;
    }

    void GOP::print(const char& c, Color color, Color background) {
        switch (c) {
            case '\0': 
                return;
            case '\n':
                ++cursor_cell_Y;
                [[fallthrough]];
            case '\r':
                cursor_cell_X = 0;
                break;
            case '\t':
                cursor_cell_X = (cursor_cell_X + 4) & ~3;
                break;
            case '\b':
                if (cursor_cell_X == 0) {
                    if (cursor_cell_Y != 0) {
                        --cursor_cell_Y;
                        cursor_cell_X = CELL_NUM_X - 1;
                    }
                }
                else {
                    --cursor_cell_X;
                }
                displayGlyph(cursor_cell_X++ * GLYPH_COL_COUNT, cursor_cell_Y * GLYPH_ROW_COUNT, ' ', color, background); 
                break;
            default: displayGlyph(cursor_cell_X++ * GLYPH_COL_COUNT, cursor_cell_Y * GLYPH_ROW_COUNT, c, color, background); break;
        }

        if (cursor_cell_X >= CELL_NUM_X) {
            cursor_cell_X = 0;
            ++cursor_cell_Y;
        }
        if (cursor_cell_Y >= CELL_NUM_Y) {
            cursor_cell_Y = CELL_NUM_Y - 1;
            for (uint32_t x = 0; x < CELL_NUM_X; ++x) {
                displayGlyph(x * GLYPH_COL_COUNT, cursor_cell_Y * GLYPH_ROW_COUNT, ' ', { 255, 255, 255 }, { 0, 0, 0 }, true);
            }
        }
    }

    void GOP::print(const char* str, Color color, Color background) {
        for (char c = *str; c != '\0'; c = *(++str)) {
            print(c, color, background); 
        }
    }

    void GOP::printIntegral(const uint64_t& n) {
        if (n == 0) {
            GOP::print('0');
            return;
        }

        char num[20] = {};
        int index = 0;

        uint64_t num_cpy = n;
        while (num_cpy != 0) {
            num[index++] = '0' + (num_cpy % 10);
            num_cpy /= 10;
        }

        for (int i = index - 1; i >= 0; --i) {
            print(num[i]);
        }
 
    }

    void GOP::printIntegral(const int64_t& n) {
        if (n < 0) {
            print('-');
            print(static_cast<uint64_t>(-(n + 1)) + 1);
            return;
        }
        print(static_cast<uint64_t>(n));
    }

    void GOP::print(bool n) {
        GOP::print(n ? "True" : "False");
    }

    void GOP::printHex(uint64_t n) {
        print("0x");
        for (int i = 15; i >= 0; --i) {
            uint8_t digit = (n >> (i * 4)) & 0xF;
            print(static_cast<char>(digit < 10 ? '0' + digit : 'A' + (digit - 10)));
        }
    }

}
