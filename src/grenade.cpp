#include "grenade.h"

#include "func_wrapper.h"
#include "time_interface.h"
#include "variable.h"
#include "vtbl.h"
#include "wds.h"

static Var<grenade *> active_grenades{0x0095C76C};

grenade::grenade(const string_hash &a2, unsigned int a3) : actor(a2, a3)
{
    THISCALL(0x00536580, this, &a2, a3);
}

void grenade::frame_advance_all_grenades(Float elapsed)
{
    for (auto *current = active_grenades(); current != nullptr;) {
        auto *next = *reinterpret_cast<grenade **>(
            reinterpret_cast<char *>(current) + 0xC0);
        const float scale = current->field_58 != nullptr
            ? static_cast<float>(current->field_58->sub_4ADE50())
            : g_world_ptr->field_158.field_0;
        if (current->m_vtbl != 0) {
            auto *address = get_vfunc(current->m_vtbl, 0x1A4);
            if (address != nullptr) {
                void(__fastcall *frame_advance)(grenade *, void *, Float) =
                    CAST(frame_advance, address);
                frame_advance(current, nullptr, Float{scale * elapsed.value});
            }
        }
        current = next;
    }
}

void grenade::sub_4D6B10(int a2)
{
    THISCALL(0x004D6B10, this, a2);
}
