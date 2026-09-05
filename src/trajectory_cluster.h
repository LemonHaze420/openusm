#pragma once

#include "variable.h"
#include "fixed_pool.h"


struct trajectory_cluster_t {
    trajectory_cluster_t();

    static inline Var<fixed_pool> pool{0x0};
};
