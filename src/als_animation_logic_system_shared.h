#pragma once

#include "mvector.h"

struct mash_info_struct;

namespace als {

struct state_machine_shared;
struct layer_state_machine_shared;
struct als_meta_anim_table_shared;

struct animation_logic_system_shared {
    mVector<als::layer_state_machine_shared> field_0;
    state_machine_shared *field_14;
    als_meta_anim_table_shared *field_18;

    //0x004AC000
    animation_logic_system_shared();

    void destruct_mashed_class();

    void unmash(mash_info_struct *a1, void *a3);
};
} // namespace als

extern void animation_logic_sytem_shared_patch();
