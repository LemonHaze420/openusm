#include "attack_impact_sound_event.h"

#include "common.h"
#include "func_wrapper.h"

#include <algorithm>
#include <array>

VALIDATE_SIZE(attack_impact_sound_event, 0x18);

namespace {
void __fastcall destruct_impact(attack_impact_sound_event *self, void *)
{
    self->sound.destruct_mashed_class();
    self->field_4.destruct_mashed_class();
}
void __fastcall unmash_impact(attack_impact_sound_event *self, void *, mash_info_struct *info, void *)
{
    self->field_4.unmash(info, self);
    self->sound.unmash(info, self);
}
unsigned __fastcall impact_type(const attack_impact_sound_event *)
{
    return 553;
}
bool __fastcall impact_subclass(const attack_impact_sound_event *, void *, unsigned type)
{
    return type == 539 || type == 573;
}
bool __fastcall impact_is_or_subclass(const attack_impact_sound_event *, void *, unsigned type)
{
    return type == 553 || type == 539 || type == 573;
}
int __fastcall impact_size(const attack_impact_sound_event *)
{
    return sizeof(attack_impact_sound_event);
}
}

void *attack_impact_sound_event::native_vtable()
{
    static auto table = [] {
        std::array<void *, 8> result;
        std::copy_n(static_cast<void **>(event::native_vtable()), result.size(), result.data());
        result[0] = bit_cast<void *>(&destruct_impact);
        result[1] = bit_cast<void *>(&unmash_impact);
        result[3] = bit_cast<void *>(&impact_type);
        result[4] = bit_cast<void *>(&impact_subclass);
        result[5] = bit_cast<void *>(&impact_is_or_subclass);
        result[7] = bit_cast<void *>(&impact_size);
        return result;
    }();
    return table.data();
}

attack_impact_sound_event::attack_impact_sound_event()
    : event(STORED_ATTACK_IMPACT_SOUND), sound{0}, volume{1.0f}, source{0}
{
    if constexpr (STANDALONE_SYSTEM)
        m_vtbl = bit_cast<std::intptr_t>(native_vtable());
    else
        THISCALL(0x00579300, this);
}

attack_impact_sound_event::attack_impact_sound_event(from_mash_in_place_constructor *tag)
    : event(STORED_ATTACK_IMPACT_SOUND), sound{0}, volume{1.0f}, source{0}
{
    if constexpr (STANDALONE_SYSTEM)
        m_vtbl = bit_cast<std::intptr_t>(native_vtable());
    else
        THISCALL(0x00579330, this, tag);
}
