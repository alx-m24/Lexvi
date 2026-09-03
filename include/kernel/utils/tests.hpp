#pragma once

#ifndef NDEBUG

#define KERNEL_TEST_FUNC(func_name, return_type, args, body) \
    inline return_type func_name args body

#else

#define KERNEL_TEST_FUNC(func_name, return_type, args, body) \
    [[maybe_unused]] inline bool func_name args { return true; }

#endif
