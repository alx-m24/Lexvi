#include "kernel/debug/gop.hpp"

#include "kernel/kernel-config.hpp"
#include "kernel/memory/memory-defs.hpp"

#include "kernel/error/error.hpp"

namespace kernel {
    GOP gop{};

    void load_GOP() {
        gop = *reinterpret_cast<GOP*>(TO_VIRT(GOP_PHYS_ADDRESS));
        gop.Info = reinterpret_cast<GOP_Info*>(TO_VIRT(gop.Info));
    }

    void init_FrameBuffer() {
        gop.FrameBufferBase = MMIO_TO_VIRT(gop.FrameBufferBase);
    }

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
        
        // Pack r,g,b into a pixel value matching the GOP's actual format.
        uint32_t pack_pixel(const GOP_Info& info, uint8_t r, uint8_t g, uint8_t b) {
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
    } // namespace
    
    void gop_test() {
        KERNEL_ASSERT(gop.Info || gop.FrameBufferBase != 0);
        KERNEL_ASSERT(gop.Info->PixelFormat != GOP_PixelFormat::PixelBltOnly);
    
        uint32_t* fb = reinterpret_cast<uint32_t*>(gop.FrameBufferBase);
    
        const uint32_t width  = gop.Info->HorizontalResolution;
        const uint32_t height = gop.Info->VerticalResolution;
        const uint32_t pitch  = gop.Info->PixelsPerScanLine;
    
        for (uint32_t y = 0; y < height; ++y) {
            uint32_t* row = fb + y * pitch;
            for (uint32_t x = 0; x < width; ++x) {
                // Hue sweeps 0-360 across the width, like a UV.x gradient in a fullscreen shader.
                float hue = (static_cast<float>(x) / static_cast<float>(width)) * 360.0f;
    
                uint8_t r, g, b;
                hsv_to_rgb(hue, 1.0f, 1.0f, r, g, b);
    
                row[x] = pack_pixel(*gop.Info, r, g, b);
            }
        }
    }
}
