#include "pulse_sum_normal.h"

#include "func_wrapper.h"

#include <cassert>

void pulse_sum_normal::set(rigid_body *b1, const math::VecClass<3, 0, void, void, math::Rep_Std<false>> &a3,
                           rigid_body *b2, const math::VecClass<3, 0, void, void, math::Rep_Std<false>> &a5,
                           math::VecClass<3, 0, void, math::VecUnit<1>, math::Rep_Std<false>> &ud,
                           pulse_sum_cache *ps_cache, const math::VecClass<3, 0, void, void, math::Rep_Std<false>> &a8)
{
    assert(ps_cache != nullptr);
    assert(b1 != nullptr);
    assert(b2 != nullptr);

    assert(b1 != b2);

    assert(std::abs(Abs(ud) - 1.0f) < .001f);


    THISCALL(0x007AA8A0, this, b1, &a3, b2, &a5, &ud, ps_cache, &a8);
}
