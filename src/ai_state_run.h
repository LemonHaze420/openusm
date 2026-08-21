#pragma once

#include "enhanced_state.h"
#include "float.hpp"
#include "vector3d.h"

struct from_mash_in_place_constructor;
struct vector3d;

namespace ai {

struct hero_inode;

struct run_state : enhanced_state {
    vector3d field_30;
    vector3d field_3C;
    float field_48;
    int field_4C;
    hero_inode *field_50;
    int field_54;
    bool field_58;

    //0x00449BD0
    run_state(from_mash_in_place_constructor *a2);

    //0x004696B0
    bool check_for_fence_hop(Float a2, vector3d *a3);

    //0x00473650
    state_trans_messages _frame_advance(Float a2);

    static inline const string_hash default_id{to_hash("RUN")};
};
}  // namespace ai

extern void run_state_patch();
