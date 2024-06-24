#include "shadow.h"

#include "common.h"
#include "debug_user_render.h"
#include "func_wrapper.h"
#include "geometry_manager.h"
#include "os_developer_options.h"
#include "potential_shadow.h"
#include "terrain.h"
#include "trace.h"
#include "variables.h"
#include "wds.h"

VALIDATE_SIZE(shadow_t, 0x4C);

void send_shadow_projectors()
{
    TRACE("send_shadow_projectors");

    CDECL_CALL(0x0059B220);

    renderDebugStrings();

    if (1) //( j_debug_render_get_ival((debug_render_items_e)21) || SHOW_OBBS || SHOW_DISTRICTS )
    {
        auto *the_terrain = g_world_ptr->get_the_terrain();
        the_terrain->show_obbs();
    }
}

bool sub_5245F0(const vector3d &a1, Float a2) {
    return geometry_manager::world_space_frustum().sub_5CC030(a1[0], a1[1], a1[2], a2);
}

bool render_projected_shadow(
    conglomerate &a1, Float a2, const vector3d &a3, const vector3d &a4, Float radius, Float fade) {
    if constexpr (1) {
        if (os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(80)) || !g_player_shadows_enabled())
        {
            return false;
        }

        if (g_cur_shadow_target() < 2) {
            assert(fade > 0 && radius > 0);

            vector3d v17 = a4 - a3;
            if (sub_5245F0(v17, radius)) {
                potential_shadow v18;

                v18.field_18 = &a1;

                v18.field_C = a4;

                v18.field_0 = a3;

                v18.m_fade = fade;
                v18.field_28 = 0.0;
                v18.m_radius = radius;
                v18.field_1C = a2;
                v18.sub_593280();

                auto v12 = -1;
                auto v16 = 8999999500.0f;

                for (int i = 0; i < 2; ++i) {
                    auto *shadow = &shadow_candidates()[i];

                    if (shadow->field_18 == nullptr ||
                        (shadow->field_28 < v18.field_28 && shadow->field_28 < v16) ) {
                        v12 = i;
                        v16 = shadow->field_28;
                    }
                }

                if (v12 >= 0) {
                    auto &v13 = shadow_candidates()[v12];

                    v13.sub_5932C0();
                    v13 = v18;

                    return true;
                }
            }

            return true;
        }

        return false;
    } else {
        return (bool) CDECL_CALL(0x0059B090, &a1, a2, &a3, &a4, radius, fade);
    }
}
