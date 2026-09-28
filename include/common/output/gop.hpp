#pragma once

#include <cstdint>
#include <concepts>
#include <type_traits>

#include "common/types/result.hpp"

namespace Lexvi::Output {
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

    struct GOP_Data {
        uint32_t MaxMode{};
        uint32_t Mode{};
        GOP_Info* Info{};
        uint64_t SizeOfInfo{};
        uint64_t FrameBufferBase{};
        uint64_t FrameBufferSize{};
    };

    struct GOP {
        struct Color {
            uint8_t r{}; 
            uint8_t g{}; 
            uint8_t b{}; 

            Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

            constexpr uint32_t operator()(const GOP_Info& info) const;
        };

        static Types::Result Init();
        static void Shutdown();

        static void reset();

        static void print(const char& c, Color color = { 255, 255, 255 }, Color background = { 0, 0, 0 });
        static void print(const char* c, Color color = { 255, 255, 255 }, Color background = { 0, 0, 0 });
        static void printIntegral(const uint64_t& n);
        static void printIntegral(const int64_t& n);
    
        template<std::integral T>
            requires (!std::same_as<T, char> && !std::same_as<T, bool>)
        static void print(T n) {
            if constexpr (std::is_signed_v<T>)
                printIntegral(static_cast<int64_t>(n));
            else
                printIntegral(static_cast<uint64_t>(n));
        }


        static void print(bool n);

        static void printHex(uint64_t n);

        static void print() {}

        template<typename First, typename... Others>
        static void print(const First& first, const Others&... others) {
            GOP::print(first);
            GOP::print(others...);
        } 

        static void test();
        static void fill(GOP::Color color);
        static void fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, GOP::Color color);
        static void drawPixel(uint32_t x, uint32_t y, GOP::Color color); 
        static void displayGlyph(uint32_t x, uint32_t y, char c, GOP::Color color, GOP::Color background = { 0, 0, 0 }, bool overWriteBackground = false);
    };

    extern GOP_Data gop;
}
