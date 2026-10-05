#include "manip_obj.h"

#include "func_wrapper.h"
#include "entity.h"
#include "fx_cache.h"
#include "time_interface.h"
#include "variable.h"
#include "vtbl.h"
#include "wds.h"

manip_obj::manip_obj() {}

void manip_obj::frame_advance_all_manip_objs(Float elapsed)
{
    static auto &active = var<entity *>(0x0095A6C4);
    static auto &inactive = var<entity *>(0x0095A6C0);

    for (auto *current = active; current != nullptr;) {
        auto *next = *reinterpret_cast<entity **>(
            reinterpret_cast<char *>(current) + 0xC0);
        const float scale = current->field_58 != nullptr
            ? static_cast<float>(current->field_58->sub_4ADE50())
            : g_world_ptr->time_manager.field_0;
        if (current->m_vtbl != 0) {
            auto *address = get_vfunc(current->m_vtbl, 0x1A4);
            if (address != nullptr) {
                void(__fastcall *frame_advance)(entity *, void *, Float) =
                    CAST(frame_advance, address);
                frame_advance(current, nullptr, Float{scale * elapsed.value});
            }
        }
        current = next;
    }

    for (auto *current = inactive; current != nullptr;) {
        auto *next = *reinterpret_cast<entity **>(
            reinterpret_cast<char *>(current) + 0xC0);
        auto *cache = *reinterpret_cast<fx_cache **>(
            reinterpret_cast<char *>(current) + 0x138);
        if (cache != nullptr) {
            const float scale = current->field_58 != nullptr
                ? static_cast<float>(current->field_58->sub_4ADE50())
                : g_world_ptr->time_manager.field_0;
            cache->frame_advance(Float{scale * elapsed.value});
        }
        current = next;
    }
}
