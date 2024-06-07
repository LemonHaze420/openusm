#pragma once

#include "mashable_vector.h"

struct anim_info;
struct generic_mash_header;
struct generic_mash_data_ptrs;

struct anim_map_ptr_entry
{
    int field_0;
    int field_4;
    mashable_vector<anim_info> field_8;

    void un_mash(
        generic_mash_header *a2,
        void *a3,
        generic_mash_data_ptrs *a4);
};
