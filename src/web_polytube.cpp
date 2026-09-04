#include "web_polytube.h"

#include "common.h"
#include "swinger.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

VALIDATE_SIZE(web_polytube, 0x17C);

web_polytube::web_polytube(swinger_t *a1, const string_hash &a2, uint32_t a3) : polytube(a2, a3)
{
    this->m_vtbl = 0x0087E738;
    this->field_178 = a1;
}

void web_polytube::render(Float a2)
{
    TRACE("web_polytube::render:");

    if constexpr (1) {
        if (this->field_178 != nullptr) {
            this->rebuild_web();
        }

        polytube::render(a2);
    }
}

void web_polytube::rebuild_web()
{
    if constexpr (1) {
        auto *swinger = this->field_178;

        sp_log("rebuild_web: visual_point = %s, ", swinger->m_visual_point.to_string().c_str());

        const vector3d &a2 = swinger->m_visual_point;
        auto &v38 = swinger->field_0;
        const vector3d v37 = swinger->field_3C->get_abs_position() + swinger->field_40;

        auto v2 = this->get_num_control_pts() - 1;
        this->set_abs_control_pt(v2, a2);
        this->set_abs_control_pt(0, v37);

        auto v4 = v37 * 3.0f + a2 * 2.0f;
        const auto v34 = v4 / 5.0f;
        auto v6 = v37 - v38.get_pivot_abs_pos();
        auto v18 = (v38.get_constraint() - v6.length() - 0.5f) / 2.0f;
        auto a3 = (v38.get_constraint() - 9.0f) / 9.0f + v18;
        if (a3 < LARGE_EPSILON) {
            a3 = 0.0f;
        }

        const auto v16 = YVEC * a3;
        auto v9 = (v37 + v34) / 2.0f - v16;
        this->set_abs_control_pt(1, v9);

        auto v12 = (a2 + v34) / 2.0f - v16;
        this->set_abs_control_pt(2, v12);
        this->rebuild_helper();

    } else {
        THISCALL(0x004775A0, this);
    }
}

void web_polytube_patch()
{
    {
        FUNC_ADDRESS(address, &web_polytube::render);
        //SET_JUMP(0x, address);
    }
}
