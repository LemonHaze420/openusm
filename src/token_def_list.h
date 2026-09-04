#pragma once

#include "mash.h"
#include "mvector.h"

struct token_def;
struct mash_info_struct;
struct from_mash_in_place_constructor;

struct token_def_list {
    mVector<token_def> field_0;
    int field_14[9];

    //0x005DEDA0
    token_def_list(from_mash_in_place_constructor *a2);

    void initialize(mash::allocation_scope a2);

    void clear();

    //0x005DD200
    void unmash(mash_info_struct *a2, void *a3);
};

extern void token_def_list_patch();
