#pragma once

#include "attach_action_trigger_enum.h"
#include "mash.h"
#include "mvector.h"
#include "mVectorBasic.h"
#include "mstring.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

struct attach_node {
    mVectorBasic<attach_action_trigger_enum> field_0;
    mString field_10;
    int field_20;
    int field_24;

    attach_node(from_mash_in_place_constructor *a2);

    void initialize(mash::allocation_scope a2);

    void unmash(mash_info_struct *a1, void *);
};
