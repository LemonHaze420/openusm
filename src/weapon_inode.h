#pragma once

#include "info_node.h"
#include "mvector.h"
#include "weapon_instance.h"

namespace ai {

struct ai_core;

struct weapon_inode : info_node {
    mVector<ai::weapon_instance> field_1C;
    int field_30;
    int field_34;

    static string_hash default_id;
    static void *native_vtable();

    weapon_inode();
    weapon_inode(from_mash_in_place_constructor *);
    ~weapon_inode();

    void _unmash(mash_info_struct *, void *);
    void destruct_mashed_class();
    void destroy_weapons();
    void clear_weapons();
    vhandle_type<handheld_item> get_weapon_handle(uint16_t index) const;

    //0x006CD5A0
    void activate(ai_core *a2);

    //0x006CC150
    void create_weapons();
};
}  // namespace ai
