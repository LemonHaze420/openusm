#pragma once

#include "mstring.h"
#include "resource_key.h"
#include "string_hash.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

struct tracking_panel {
    mString field_0;
    resource_key field_10;
    string_hash field_18;
    char field_1C[0x60];

    tracking_panel(from_mash_in_place_constructor *a2);
    void unmash(mash_info_struct *a1, void *a3);
};
