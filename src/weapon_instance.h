#pragma once

#include "mash.h"
#include "handheld_item.h"

struct actor;
struct combo_system_weapon;
struct from_mash_in_place_constructor;

namespace ai {

struct weapon_instance {
    vhandle_type<handheld_item> handle;
    const combo_system_weapon *definition;

    weapon_instance(from_mash_in_place_constructor *a2);
    weapon_instance(const combo_system_weapon *, actor *);
    ~weapon_instance();
    void destruct_mashed_class();

    //0x006C8D80
    void initialize(mash::allocation_scope a2, const combo_system_weapon *a3, actor *a4);
};

}  // namespace ai
