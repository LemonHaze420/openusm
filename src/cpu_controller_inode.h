#pragma once

#include "controller_inode.h"
#include "info_node.h"

struct from_mash_in_place_constructor;

namespace ai {

struct combat_inode;

struct cpu_controller_inode : controller_inode {
    int field_24[50];
    unsigned int pending_trigger;
    combat_inode *combat;
    bool hero_combat;
    char field_F5[3];
    int combo_chain_index;
    int next_chain_index;
    float level_time;
    bool field_104;
    char field_105[3];


    cpu_controller_inode();

    //0x00481450
    cpu_controller_inode(from_mash_in_place_constructor *a2);
    static void *native_vtable();
    void _frame_advance(Float time_step);
    vector3d _get_axis(eControllerAxis axis);
    bool has_more_moves_in_chain();
    void engage_current_move();
    void stop_chain();

    void _activate(ai_core *core);
    unsigned int get_combat_trigger(vector3d direction);
    void set_combat_trigger(unsigned int trigger);
    bool has_pending_trigger() const;
    vector2d _get_axis_2d(eControllerAxis axis);

    //0x00467FA0
    //virtual
    [[nodiscard]] game_button get_button(controller_inode::eControllerButton);
};

}  // namespace ai
