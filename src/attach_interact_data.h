#pragma once

#include "mvector.h"

struct attach_node;
struct from_mash_in_place_constructor;
struct mash_info_struct;

struct attach_interact_data
{
    mVector<attach_node> field_0;

    attach_interact_data(from_mash_in_place_constructor *a2);

    void unmash(mash_info_struct *a1, void *);
};
