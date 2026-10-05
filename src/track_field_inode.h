#pragma once

#include "info_node.h"

struct from_mash_in_place_constructor;

namespace ai {

struct track_field_inode : info_node {
    bool field_1C;
    float field_20;
    float field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
    int field_38;

    //0x00443130
    track_field_inode();

    //0x006A2280
    track_field_inode(from_mash_in_place_constructor *a2);
    static void *native_vtable();
    void _frame_advance(Float delta);
    int get_curr_button_press();
    void compute_strength_value(Float delta);
    void ui_init();
    void ui_update();

    void _activate(ai_core *core);
    void _deactivate();
};

}  // namespace ai
