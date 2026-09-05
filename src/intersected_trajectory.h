#pragma once

#include "variable.h"
#include "fixed_pool.h"


struct intersected_trajectory_t {
    intersected_trajectory_t();

    static inline Var<fixed_pool> pool{0x00937580};
};
