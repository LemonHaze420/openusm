#pragma once

#include "mash.h"
#include "mstring.h"
#include "vector3d.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;
struct region;

struct token_def {
    mString field_0;
    vector3d field_10;
    int type;
    int field_20;
    int token_id;
    region *field_28;
    void *map_dot;

    token_def(from_mash_in_place_constructor *a2);

    ~token_def();

    void initialize(mash::allocation_scope a2);

    void finalize(mash::allocation_scope a2);

    void destruct_mashed_class();

    void clear();

    void show_dot(bool a2);

    void unmash(mash_info_struct *a2, void *a3);
};
