#pragma once

#include "mstring.h"
#include "string_hash.h"
struct from_mash_in_place_constructor;
struct mash_info_struct;

struct camera_setup_entry {
    mString field_0;
    mString field_10;
    string_hash field_20;
    char field_24[0x14];

    camera_setup_entry(from_mash_in_place_constructor *a2);
    void unmash(mash_info_struct *a1, void *a3);
};
