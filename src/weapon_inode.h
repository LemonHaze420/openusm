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

    weapon_inode();

    //0x006CD5A0
    void activate(ai_core *a2);

    //0x006CC150
    void create_weapons();
};
}  // namespace ai
