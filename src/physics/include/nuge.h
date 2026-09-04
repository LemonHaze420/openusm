#pragma once

#include "ngl_math.h"

struct rigid_body;

struct nuge {
    static void get_ballistic_info(rigid_body *const *a1, int a2, math::VecClass<3, 1> *a3, math::VecClass<3, 0> *a4,
                                   float *a5);
};
