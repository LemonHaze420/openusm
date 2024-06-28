#include "weapon_instance.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(weapon_instance, 0x8);

weapon_instance::weapon_instance(from_mash_in_place_constructor *)
{
    this->field_0 = 0;
    this->initialize(mash::FROM_MASH, nullptr, nullptr);
}

void weapon_instance::initialize(mash::allocation_scope a2,
                                const combo_system_weapon *a3,
                                actor *a4)
{
    THISCALL(0x006C8D80, this, a2, a3, a4);
}

} // namespace ai
