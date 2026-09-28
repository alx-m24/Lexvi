#pragma once

#ifndef NDEBUG
#include "common/error/panic.hpp"
#define LEXVI_ASSERT(cond) \
        if (!(cond)) { LEXVI_PANIC("Assertion Failed: " #cond); }
#else
#define LEXVI_ASSERT(cond) do { } while (fasle)
#endif
