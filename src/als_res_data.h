#pragma once

#include "mash.h"
#include "resource_key.h"

struct mash_info_struct;

namespace als {
struct animation_logic_system;
}

struct als_res_data {
    resource_key field_0;
    als::animation_logic_system *field_8;
    int field_C;

    als_res_data();

    void initialize(mash::allocation_scope);

    void unmash(mash_info_struct *a2, void *a3);

    void sub_4AB7F0(int a2);

    void destruct_mashed_class();
};

extern void als_res_data_patch();
