#pragma once

#include "info_node.h"

namespace ai {

struct fall_death_inode : info_node {
    bool allow_fall_death;
    bool allow_house_death;
    bool allow_water_death;
    bool allow_obb_death;
    float terminal_velocity;
    float terminal_time;
    bool death_requested;
    uint8_t field_29[3];
    float falling_time;
    float previous_height;
    int check_index;

    fall_death_inode();
    explicit fall_death_inode(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_core *core);
    void _frame_advance(Float elapsed);
};

}
