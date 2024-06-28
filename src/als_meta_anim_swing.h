#pragma once

#include "als_meta_anim_base.h"
#include "mash.h"
#include "mvector.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;
struct nalAnyPose;

namespace als {

struct meta_key_anim;

struct als_meta_anim_swing : als_meta_anim_base {
    mVector<meta_key_anim> field_28;
    nalAnyPose *field_3C[12];
    int field_6C;

    als_meta_anim_swing();

    als_meta_anim_swing(from_mash_in_place_constructor *);

    void initialize(mash::allocation_scope a2);

    //virtual
    void _unmash(mash_info_struct *, void *);

    //virtual
    float get_anim_duration() const;

};

}

extern void als_meta_anim_swing_patch();
