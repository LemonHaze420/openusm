#pragma once

#include <cstdint>

struct ai_quad_path_exit {
    std::uint16_t district;
    std::uint16_t path;
    std::uint16_t cell;
    std::uint16_t edge;
    std::uint16_t flags;
    std::uint16_t reserved;
};
static_assert(sizeof(ai_quad_path_exit) == 0xC);
