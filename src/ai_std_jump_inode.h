#pragma once

#include "info_node.h"
#include "vector3d.h"

namespace ai {

struct ai_std_jump_inode : info_node {
    float field_1C;
    vector3d field_20;
    int field_2C;
    vector3d field_30;
    bool field_3C;
    bool field_3D;
    uint8_t field_3E[2];
    float field_40;
    float field_44;
    string_hash field_48;
    float field_4C;

    explicit ai_std_jump_inode(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _destruct_mashed_class();
    void _unmash(mash_info_struct *info, void *context);
    void _activate(ai_core *core);
    void _frame_advance(Float elapsed);
    void set_jump_velocity(const vector3d &direction, float speed);
    void set_jump_target(const vector3d &start, const vector3d &target, float max_y);
    void set_jump_parameters(bool attack, float damage, float radius, string_hash category);
};

}
