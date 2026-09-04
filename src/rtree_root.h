#pragma once

#include "ngl_math.h"

struct rtree_root_t {
    math::VecClass<4, -1> field_0;
    math::VecClass<4, -1> field_10;
    float field_20;
    float field_24;
    float field_28;
    float field_2C;

    rtree_root_t();
};
