#pragma once

#include "controller_inode.h"

struct from_mash_in_place_constructor;

namespace ai {

struct player_controller_inode : controller_inode {


    vector2d stick_cache[10];
    float facing_cache[10][3];


    player_controller_inode();

    //0x004813F0
    player_controller_inode(from_mash_in_place_constructor *a2);
    static void *native_vtable();

    float get_motion_force();

    void _frame_advance(Float time_step);
    void update_stick_cache();
    vector3d _facing();
    vector3d _get_axis(eControllerAxis axis);
    vector2d _get_axis_2d(eControllerAxis axis);
    bool has_pending_trigger();
    void set_combat_trigger(unsigned int trigger);

    //0x00467E10
    //virtual
    [[nodiscard]] game_button _get_button(controller_inode::eControllerButton a3);
};

}  // namespace ai


extern void player_controller_inode_patch();
