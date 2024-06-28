#pragma once

#include "mash.h"
#include "string_hash.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

struct interact_sound_entry
{
    string_hash field_0;
    int field_4;
    int field_8;
    float field_C;
    bool field_10;
    bool field_11;

    interact_sound_entry(from_mash_in_place_constructor *a1);

    void initialize(mash::allocation_scope a2);

    void unmash(mash_info_struct *a1, void *);
};
