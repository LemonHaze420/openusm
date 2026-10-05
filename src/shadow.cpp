#include "shadow.h"

#include "ngl.h"
#include "ngl_lighting.h"
#include "common.h"
#include "geometry_manager.h"
#include "os_developer_options.h"
#include "potential_shadow.h"
#include "variables.h"
#include "wds.h"

VALIDATE_SIZE(shadow_t, 0x4C);

#if STANDALONE_SYSTEM
namespace {

[[maybe_unused]] const bool shadow_tuning_initialized = [] {
    var<bool>(0x00922C5C) = true;
    var<bool>(0x00922C5D) = true;
    var<int>(0x0091E000) = 1;
    const float quality[4]{1.0f, 2.0f, 0.5f, 1.0f};
    for (int i = 0; i != 4; ++i) {
        var<float[4]>(0x00921B24)[i] = quality[i];
        var<float[4]>(0x00921B34)[i] = quality[i];
    }
    return true;
}();
}
#endif

void send_shadow_projectors()
{
    for (auto &candidate : shadow_candidates()) {
        if (candidate.field_18 != nullptr)
            candidate.commit();
    }
    for (auto &shadow : g_shadow()) {
        if (shadow.field_48 && shadow.field_44 != nullptr) {
            nglListAddDirProjectorLight(0x02000000, shadow.field_0, shadow.field_40 * 2.0f,
                                      shadow.field_40 * -2.0f, -6.5f, 0.0f, 2,
                                      0xFF000000, shadow.field_44);
        }
    }
}

bool sub_5245F0(const vector3d &a1, Float a2)
{
    return geometry_manager::world_space_frustum.sub_5CC030(a1[0], a1[1], a1[2], a2);
}

bool render_projected_shadow(conglomerate &owner, Float camera_distance, const vector3d &direction,
                             const vector3d &position, Float radius, Float fade)
{
    if (os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(80)) ||
        !g_player_shadows_enabled || g_cur_shadow_target >= 2)
        return false;
    if (sub_5245F0(position - direction, radius)) {
        potential_shadow candidate;
        candidate.field_18 = &owner;
        candidate.field_C = position;
        candidate.field_0 = direction;
        candidate.m_fade = fade.value;
        candidate.field_28 = 0.0f;
        candidate.m_radius = radius.value;
        candidate.field_1C = camera_distance.value;
        candidate.sub_593280();
        int selected = -1;
        float lowest_priority = 8999999500.0f;
        for (int i = 0; i != 2; ++i) {
            const auto &existing = shadow_candidates()[i];
            if (existing.field_18 == nullptr ||
                (existing.field_28 < candidate.field_28 && existing.field_28 < lowest_priority)) {
                selected = i;
                lowest_priority = existing.field_28;
            }
        }
        if (selected >= 0) {
            auto &destination = shadow_candidates()[selected];
            destination.sub_5932C0();
            destination = candidate;
        }
    }
    return true;
}
