#pragma once

#include "als_meta_anim_base.h"

#include "mash.h"
#include "nal_anim.h"
#include "mvector.h"
#include "string_hash.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

struct nalAnyPose;

namespace ai {
struct ai_core;

struct meta_anim_interact : als::als_meta_anim_base {
    int field_28;
    int field_2C;

    meta_anim_interact();

    //virtual
    void _unmash(mash_info_struct *, void *);
};

struct strength_test_anim_inst;

struct meta_anim_strength_test : als::als_meta_anim_base {
    nalAnimClass<nalAnyPose> *field_28;
    nalAnimClass<nalAnyPose> *field_2C;
    bool field_30;
    ai_core *field_34;

    meta_anim_strength_test();

    meta_anim_strength_test(from_mash_in_place_constructor *a2);

    void initialize(mash::allocation_scope a2);

    //virtual
    int _get_virtual_type_enum() const;

    //virtual
    void _unmash(mash_info_struct *, void *);

    //0x00451990
    //virtual
    bool _is_anim_looping() const;

    //0x004519B0
    //virtual
    bool _is_anim_trajectory_relative() const;

    //0x004519D0
    //virtual
    float _get_anim_duration() const;

    //0x004519F0
    //virtual
    nalBaseSkeleton *_get_skeleton();

    //0x00451A00
    //virtual
    strength_test_anim_inst *_create_anim_inst(nalBaseSkeleton *a2, nalAnimClass<nalAnyPose> *a3,
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

struct strength_test_anim_inst : nalAnimClass<nalAnyPose>::nalInstanceClass {
    void *field_14;
    void *field_18;
    ai::ai_core *field_1C;
    float field_20;

    //0x0048BA70
    strength_test_anim_inst(nalAnimClass<nalAnyPose> *a2, nalAnimClass<nalAnyPose> *a3, nalBaseSkeleton *a4,
                            ai::ai_core *a5);
};
}  // namespace ai

namespace als {
struct meta_key_anim {
    string_hash field_0;
    nalAnimClass<nalAnyPose> *field_4;
    int field_8;
    int field_C;

    meta_key_anim();

    meta_key_anim(from_mash_in_place_constructor *a2);

    void initialize(mash::allocation_scope a2);

    void clear();

    void lookup_nal_animation();

    void unmash(mash_info_struct *a1, void *);
};

struct als_meta_linear_blend : als_meta_anim_base {
    struct nalInstance : nalAnimClass<nalAnyPose>::nalInstanceClass {
        void **field_14;
        animation_logic_system *field_18;
        state_machine *field_1C;
        als_meta_linear_blend *field_20;

        nalInstance(als::als_meta_linear_blend *a2, nalBaseSkeleton *a3, als::animation_logic_system *a4,
                    als::state_machine *a5);
    };

    mVector<als::meta_key_anim> key_anims;
    int field_3C;

    als_meta_linear_blend();

    als_meta_linear_blend(from_mash_in_place_constructor *a2);

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

    //0x0049E680
    //virtual
    als_meta_linear_blend::nalInstance *_create_anim_inst(nalBaseSkeleton *a2, nalAnimClass<nalAnyPose> *a3,
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

extern void meta_anim_interact_patch();
