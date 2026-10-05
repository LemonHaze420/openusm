#include "light_manager.h"

#include "func_wrapper.h"
#include "common.h"
#include "entity.h"
#include "region.h"
#include "region_mash_info.h"
#include "terrain.h"
#include "time_interface.h"
#include "wds.h"

#include <cmath>

VALIDATE_SIZE(light_manager, 0x34);

light_manager::light_manager() {}

light_manager::light_manager(int)
{
    auto *v72 = this;
    v72->field_0 = 0;
    v72->field_C = 0;

    v72->field_10.r = 0.0;
    v72->field_10.g = 0.0;
    v72->field_10.b = 0.0;
    v72->field_10.a = 0.0;

    v72->field_20.r = 0.0;
    v72->field_20.g = 0.0;
    v72->field_20.b = 0.0;
    v72->field_20.a = 0.0;

    v72->field_30 = 0.0;
    v72->field_8 = nullptr;
    auto *v73 = light_manager::active_light_managers();
    v72->field_4 = active_light_managers();
    if (active_light_managers() != nullptr) {
        v73->field_8 = v72;
    }

    active_light_managers() = v72;
}

void light_manager::frame_advance_all_light_managers(Float elapsed)
{
#if STANDALONE_SYSTEM
    auto *manager = active_light_managers();
    while (manager != nullptr) {
        auto *current = manager;
        manager = current->field_4;
        const vhandle_type<entity> owner_handle{entity_base_vhandle{static_cast<uint32_t>(current->field_C)}};
        auto *owner = owner_handle.get_volatile_ptr();
        if (owner == nullptr)
            continue;
        auto *primary_region = owner->get_primary_region();
        if (primary_region == nullptr)
            continue;

        using has_time_fn = bool(__fastcall *)(entity *, void *);
        using time_fn = time_interface *(__fastcall *)(entity *, void *);
        const auto has_time = reinterpret_cast<has_time_fn>(get_vfunc(owner->m_vtbl, 0x10C));
        const auto time = reinterpret_cast<time_fn>(get_vfunc(owner->m_vtbl, 0x110));
        const float time_scale = has_time(owner, nullptr) ? static_cast<float>(time(owner, nullptr)->sub_4ADE50())
                                                          : g_world_ptr->time_manager.field_0;
        current->frame_advance(primary_region, Float{elapsed.value * time_scale}, !owner->get_occluded_last_frame());
    }
#else
    CDECL_CALL(0x0053B040, elapsed);
#endif
}

void light_manager::frame_advance(region *primary_region, Float elapsed, bool interpolate)
{
#if STANDALONE_SYSTEM
    const vhandle_type<entity> owner_handle{entity_base_vhandle{static_cast<uint32_t>(field_C)}};
    auto *owner = owner_handle.get_volatile_ptr();
    auto *innermost_region = g_world_ptr->the_terrain->find_innermost_region(owner->get_abs_position());
    if (innermost_region == nullptr || (innermost_region->flags & (0x100u | 0x40000u)) != 0) {
        field_20 = primary_region->mash_info->field_20;
        field_30 += elapsed.value;
        if (field_30 > 1.0f)
            field_30 = 1.0f;
    } else {
        const auto &ambient = var<color>(0x00960140);
        field_20 = color{ambient.r, ambient.g, ambient.b, 1.0f};
        field_30 -= elapsed.value;
        if (field_30 < 0.0f)
            field_30 = 0.0f;
    }

    if (!interpolate) {
        field_10 = field_20;
        return;
    }


    const float red_delta = field_20.r - field_10.r;
    const double green_delta = double(field_20.g) - field_10.g;
    const float blue_delta = field_20.b - field_10.b;
    const float alpha_delta = field_20.a - field_10.a;
    const double distance_squared =
        double(blue_delta) * blue_delta + green_delta * green_delta + double(red_delta) * red_delta;
    const double step = double(elapsed.value) * 3.0 * double(0.6667f);
    const float rounded_step = static_cast<float>(step);
    if (distance_squared > step * rounded_step) {
        const double fraction = rounded_step / std::sqrt(distance_squared);
        field_10.r = static_cast<float>(field_10.r + red_delta * fraction);
        field_10.g += static_cast<float>(green_delta * fraction);
        field_10.b += static_cast<float>(blue_delta * fraction);
        field_10.a += static_cast<float>(alpha_delta * fraction);
    } else {
        field_10 = field_20;
    }
#else
    THISCALL(0x00534980, this, primary_region, elapsed, interpolate);
#endif
}

void light_manager::remove_from_list()
{
    if constexpr (STANDALONE_SYSTEM) {
        if (field_8 != nullptr)
            field_8->field_4 = field_4;
        else if (active_light_managers() == this)
            active_light_managers() = field_4;
        if (field_4 != nullptr)
            field_4->field_8 = field_8;
        field_4 = nullptr;
        field_8 = nullptr;
    } else {
        THISCALL(0x00515E70, this);
    }
}
