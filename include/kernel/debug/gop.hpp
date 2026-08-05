#pragma once

#include <cstdint>

namespace kernel {
    enum class GOP_PixelFormat : uint32_t {
        PixelRedGreenBlueReserved8BitPerColor = 0,
        PixelBlueGreenRedReserved8BitPerColor = 1,
        PixelBitMask = 2,
        PixelBltOnly = 3
    };

     struct GOP_Info {
         uint32_t Version{};
         uint32_t HorizontalResolution{};
         uint32_t VerticalResolution{};
         GOP_PixelFormat PixelFormat{};
         uint32_t PixelInformation[4]{};
         uint32_t PixelsPerScanLine{};
     };

    struct GOP {
        uint32_t MaxMode{};
        uint32_t Mode{};
        GOP_Info* Info{};
        uint64_t SizeOfInfo{};
        uint64_t FrameBufferBase{};
        uint64_t FrameBufferSize{};
    };

    extern GOP gop;

    void load_GOP();
    void init_FrameBuffer();
    void gop_test();
}
