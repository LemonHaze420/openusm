#pragma once

#include "mash.h"

struct actor;
struct combo_system_weapon;
struct from_mash_in_place_constructor;

namespace ai {

struct weapon_instance {
    int field_0;
    int field_4;

    weapon_instance(from_mash_in_place_constructor *a2);

    //0x006C8D80
    void initialize(mash::allocation_scope a2, const combo_system_weapon *a3, actor *a4);
};

} // namespace ai
