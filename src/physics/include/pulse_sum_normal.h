#pragma once

#include "ngl_math.h"

struct pulse_sum_cache;
struct rigid_body;

struct pulse_sum_normal {
    void set(rigid_body *b1, const math::VecClass<3, 0, void, void, math::Rep_Std<false>> &a3, rigid_body *b2,
             const math::VecClass<3, 0, void, void, math::Rep_Std<false>> &a5,
             math::VecClass<3, 0, void, math::VecUnit<1>, math::Rep_Std<false>> &ud, pulse_sum_cache *ps_cache,
             const math::VecClass<3, 0, void, void, math::Rep_Std<false>> &a8);
};
