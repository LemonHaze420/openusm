#pragma once

#include "ngl_math.h"
#include <cstdint>

struct rtree_node_t;

struct rtree_root_t {
    math::VecClass<4, -1> field_0;
    math::VecClass<4, -1> field_10;
    rtree_node_t *field_20;
    std::uintptr_t field_24;
    uint32_t field_28;
    uint32_t field_2C;

    rtree_root_t();
};
