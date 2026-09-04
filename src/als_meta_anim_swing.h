#pragma once

#include "als_meta_anim_base.h"
#include "mash.h"
#include "mvector.h"
#include "sampling_window.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;
struct nalAnyPose;

namespace nalComp {
struct nalCompSkeleton;
}

namespace als {

struct meta_key_anim;

struct als_meta_anim_swing : als_meta_anim_base {
    struct nalInstance : nalAnimClass<nalAnyPose>::nalInstanceClass {
        animation_logic_system *field_14;
        state_machine *field_18;
        als_meta_anim_swing *field_1C;
        int field_20;
        int field_24;
        sampling_window field_28;

        nalInstance(als_meta_anim_swing *a2, nalBaseSkeleton *a3, animation_logic_system *a4, state_machine *a5);
    };

    mVector<meta_key_anim> field_28;
    nalAnyPose *field_3C[12];
    int field_6C;

    als_meta_anim_swing();

    als_meta_anim_swing(from_mash_in_place_constructor *);

    //0x004A0350
    void initialize(mash::allocation_scope a2);

    nalAnimClass<nalAnyPose> *get_anim_proxy() const;

    //virtual
    void _unmash(mash_info_struct *, void *);

    //virtual
    int _get_virtual_type_enum() const;

    //virtual
    bool _is_anim_looping() const;

    //virtual
    bool _is_anim_trajectory_relative() const;

    //virtual
    float _get_anim_duration() const;

    //virtual
    nalBaseSkeleton *_get_skeleton();

    //virtual
    als_meta_anim_swing::nalInstance *_create_anim_inst(nalBaseSkeleton *a2, nalAnimClass<nalAnyPose> *a3,
                                                        als::animation_logic_system *a4, als::state_machine *a5);

    //virtual
    int _get_mash_sizeof() const;

    static inline void *g_vtbl[] = {
        nullptr,
        func_address(&_unmash),
        nullptr,
        func_address(&_get_virtual_type_enum),
        nullptr,
        func_address(&_is_or_is_subclass_of),
        func_address(&_get_anim_name),
        func_address(&_is_anim_looping),
        func_address(&_is_anim_trajectory_relative),
        func_address(&_get_anim_duration),
        func_address(&_get_skeleton),
        func_address(&_create_anim_inst),
        nullptr,
        nullptr,
        func_address(&_get_mash_sizeof),
    };
};

}  // namespace als

extern void als_meta_anim_swing_patch();
