#pragma once

namespace Lexvi::Error {
    void panic(const char* msg, const char *file, int line);
}

#define LEXVI_PANIC(msg) Lexvi::Error::panic(msg, __FILE__, __LINE__)
