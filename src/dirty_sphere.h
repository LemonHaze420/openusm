#pragma once

#include "fixed_pool.h"
#include "sphere.h"
#include "variable.h"

struct intraframe_trajectory_t;
struct dirty_sphere_t {
    sphere bounds;
    dirty_sphere_t *next;
    intraframe_trajectory_t *trajectory;

    dirty_sphere_t() = default;
    static inline Var<fixed_pool> pool{0x009375C8};
};
