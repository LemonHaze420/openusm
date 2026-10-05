#pragma once

#include "als_meta_anim_base.h"

struct from_mash_in_place_constructor;
struct nalAnyPose;
struct actor;

template <typename T>
struct nalAnimClass;

namespace als {

struct meta_aimed_shot_vert : als_meta_anim_base {
    nalAnimClass<nalAnyPose> *field_28;
    void *field_2C;

    meta_aimed_shot_vert();

    meta_aimed_shot_vert(from_mash_in_place_constructor *a2);

    static void *native_vtable();


    void _destruct_mashed_class();
    meta_aimed_shot_vert *_scalar_deleting_destructor(uint32_t flags);
    bool _is_subclass_of(mash::virtual_types_enum type) const;
    bool _requires_delay_create() const;
    void _delay_create(actor *owner);

    //0x004512D0
    //virtual
    void _unmash(mash_info_struct *a2, void *a3);

    //virtual
    int _get_virtual_type_enum() const;

    //virtual
    bool _is_anim_looping() const;

    //0x004518B0
    //virtual
    bool _is_anim_trajectory_relative() const;

    //virtual
    float _get_anim_duration() const;

    //0x004518F0
    //virtual
    nalBaseSkeleton *_get_skeleton();

    //0x00451900
    //virtual
    nalAnimClass<nalAnyPose>::nalInstanceClass *_create_anim_inst(nalBaseSkeleton *a2, nalAnimClass<nalAnyPose> *,
                                                                  als::animation_logic_system *, als::state_machine *);

    //virtual
    int _get_mash_sizeof() const;

    static inline void *g_vtbl[] = {
        func_address(&meta_aimed_shot_vert::_destruct_mashed_class),
        func_address(&meta_aimed_shot_vert::_unmash),
        func_address(&meta_aimed_shot_vert::_scalar_deleting_destructor),
        func_address(&meta_aimed_shot_vert::_get_virtual_type_enum),
        func_address(&meta_aimed_shot_vert::_is_subclass_of),
        func_address(&mash_virtual_base::_is_or_is_subclass_of),
        func_address(&als_meta_anim_base::_get_anim_name),
        func_address(&meta_aimed_shot_vert::_is_anim_looping),
        func_address(&meta_aimed_shot_vert::_is_anim_trajectory_relative),
        func_address(&meta_aimed_shot_vert::_get_anim_duration),
        func_address(&meta_aimed_shot_vert::_get_skeleton),
        func_address(&meta_aimed_shot_vert::_create_anim_inst),
        func_address(&meta_aimed_shot_vert::_requires_delay_create),
        func_address(&meta_aimed_shot_vert::_delay_create),
        func_address(&meta_aimed_shot_vert::_get_mash_sizeof),
    };
};
}  // namespace als

extern void als_meta_aimed_shot_vert_patch();
