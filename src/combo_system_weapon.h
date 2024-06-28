#pragma once

#include "mash.h"
#include "resource_key.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

struct combo_system_weapon {
    resource_key field_0;
    resource_key field_8;
    resource_key field_10;
    string_hash field_18;
    string_hash field_1C;
    int8_t field_20;
    bool field_21;

    combo_system_weapon(from_mash_in_place_constructor *a2);

    void initialize(mash::allocation_scope a2);

    void unmash(mash_info_struct *a1, void *a3);
};
