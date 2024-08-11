#pragma once

#include "float.hpp"
#include "variable.h"

struct game_clock {
    //0x0058E2F0
    static void frame_advance(Float a1);

    static inline auto & ticks = var<Float>(0x00965AC4);
};
