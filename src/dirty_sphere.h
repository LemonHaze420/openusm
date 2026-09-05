#pragma once

#include "variable.h"
#include "fixed_pool.h"


struct dirty_sphere_t {
    dirty_sphere_t();

    static inline Var<fixed_pool> pool{0x0};
};
