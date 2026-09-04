#include "sweet_cone.h"

#include "ai_find_best_swing_anchor.h"
#include "common.h"
#include "func_wrapper.h"

#include <cassert>
#include <cmath>
#include <cstdio>

VALIDATE_SIZE(sweet_cone_t, 0x28);

sweet_cone_t::sweet_cone_t(const swing_anchor_finder *finder, const vector3d &pos, const vector3d &a3,
                           const vector3d &a4, const vector3d &a5)
{
    if constexpr (1) {
        vector3d v24 = a4;
        v24[1] = 0.0;

        auto a1 = v24.length();
        if (a1 >= 0.2) {
            v24 = v24 / a1;
        } else {
            v24 = a3;

            v24[1] = 0.0;
        }

        this->direction = v24 * finder->field_0 + YVEC * finder->field_4;

        assert(direction.is_valid());

        assert(vector3d(0.0f, 0.0f, finder->sweet_spot_distance).is_valid());

        auto v10 = (pos + this->direction * finder->sweet_spot_distance);

        this->sweet_spot = v10 + a5;

        this->m_position = pos;

        this->direction = this->sweet_spot - this->m_position;
        this->direction.normalize();

        this->field_24 = std::cos(30.f);

    } else {
        THISCALL(0x00451D20, this, finder, &pos, &a3, &a4, &a5);
    }
}

bool sweet_cone_t::contains_point(const vector3d &a2) const
{
    vector3d v1 = a2 - this->m_position;
    v1.normalize();

    auto local_dot = dot(this->direction, v1);

    auto result = (local_dot > this->field_24);

    if constexpr (0) {
        sp_log("sweet_cone_t = %s, a2 = %s", this->to_string(), a2.to_string());
    }

    return result;
}

const char *sweet_cone_t::to_string() const
{
    static char str[200];
    sprintf(str,
            "%s, %s, %s, %.2f",
            m_position.to_string().c_str(),
            direction.to_string().c_str(),
            sweet_spot.to_string().c_str(),
            field_24);

    return str;
}
