#pragma once

#include "float.hpp"
#include "variable.h"
#include <cstdint>

struct game_clock {
    //0x0058E2F0
    static void frame_advance(Float a1);

#if STANDALONE_SYSTEM
    static inline std::uint32_t frames{};
    static inline float delta{};
    static inline std::uint64_t ticks{};
#else
    static inline auto &ticks = var<Float>(0x00965AC4);
#endif
};
