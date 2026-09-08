#include "sound_interface.h"

#include "func_wrapper.h"
#include "common.h"
#include "trace.h"
#include "utility.h"
#include "entity.h"
#include "time_interface.h"
#include "variable.h"
#include "vtbl.h"
#include "wds.h"

VALIDATE_SIZE(sound_interface, 0x28);

sound_interface::sound_interface() {}

void sound_interface::frame_advance_all_sound_ifc(Float elapsed)
{
    TRACE("sound_interface::frame_advance_all_sound_ifc");
    static auto &interfaces = var<_std::vector<sound_interface *> *>(0x0095A6A4);
    if (interfaces == nullptr) {
        return;
    }
    for (auto *interface_ptr : *interfaces) {
        if (interface_ptr == nullptr) {
            continue;
        }
        if (interface_ptr->m_vtbl == 0) {
            continue;
        }
        auto *owner = reinterpret_cast<entity *>(interface_ptr->field_4);
        const float scale = owner != nullptr && owner->field_58 != nullptr
            ? static_cast<float>(owner->field_58->sub_4ADE50())
            : g_world_ptr->field_158.field_0;
        auto *address = get_vfunc(interface_ptr->m_vtbl, 0x28);
        if (address != nullptr) {
            void(__fastcall *frame_advance)(sound_interface *, void *, Float) =
                CAST(frame_advance, address);
            frame_advance(interface_ptr, nullptr, Float{scale * elapsed.value});
        }
    }
}

void sound_interface_patch()
{
    REDIRECT(0x005584FA, sound_interface::frame_advance_all_sound_ifc);
}
