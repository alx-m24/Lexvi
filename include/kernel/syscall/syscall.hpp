#pragma once

#include "kernel/interrupt/interruptFrame.hpp"

#include <stdint.h>

uint64_t handle_syscall(const interrupt_frame_t& frame);
