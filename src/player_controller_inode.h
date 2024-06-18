#pragma once

#include "controller_inode.h"

struct from_mash_in_place_constructor;

namespace ai {

struct player_controller_inode : controller_inode {
    int field_1C[52];

    //0x004813F0
    player_controller_inode(from_mash_in_place_constructor *a2);

    float get_motion_force();

    //0x00467E10
    //virtual
    [[nodiscard]] game_button _get_button(controller_inode::eControllerButton a3);
};

} // namespace ai


extern void player_controller_inode_patch();
