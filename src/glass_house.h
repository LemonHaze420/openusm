#pragma once

#include "mVectorBasic.h"
#include "string_hash.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;
struct subdivision_node_large_obb;

struct glass_house
{
    string_hash field_0;
    mVectorBasic<subdivision_node_large_obb> obbs;

    glass_house(from_mash_in_place_constructor *a2);

    void render();

    void destruct_mashed_class();

    void unmash(mash_info_struct *a1, void *);

    string_hash get_name() const
    {
        return this->field_0;
    }
};
