#pragma once

#include "enhanced_state.h"

#include "string_hash.h"

struct vector3d;
struct po;
struct from_mash_in_place_constructor;

namespace ai {
struct interaction_inode;

struct interaction_state : enhanced_state {
    interaction_inode *field_30;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
    int field_54;

    interaction_state();

    //0x00450AC0
    interaction_state(from_mash_in_place_constructor *a2);

    //0x0047E7F0
    po *compute_interact_point(vector3d *a4, vector3d *a2, vector3d *a3);

    //0x00482840
    void move_actor_to_interact_point();

    //0x00462DF0
    //virtual
    int frame_advance(Float a2);

    static const inline string_hash default_id{static_cast<int>(to_hash("interaction"))};

    static Var<string_hash> pre_interact_motion_hash;

    static Var<string_hash> pre_interact_run_motion_hash;
};
}  // namespace ai
