#pragma once

#include "als_meta_anim_base.h"

struct from_mash_in_place_constructor;
struct nalAnyPose;

template<typename T>
struct nalAnimClass;

namespace als {

    struct meta_aimed_shot_vert : als_meta_anim_base {
        nalAnimClass<nalAnyPose> *field_28;
        void *field_2C;

        meta_aimed_shot_vert();

        meta_aimed_shot_vert(from_mash_in_place_constructor *a2);

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
        nalBaseSkeleton * _get_skeleton();

        //0x00451900
        //virtual
        nalAnimClass<nalAnyPose>::nalInstanceClass * _create_anim_inst(
                nalBaseSkeleton *a2,
                nalAnimClass<nalAnyPose> *,
                als::animation_logic_system *,
                als::state_machine *);

        //virtual
        int _get_mash_sizeof() const;

        static inline void * g_vtbl[] = {
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
            func_address(&_get_mash_sizeof)
        };
    };
}

extern void als_meta_aimed_shot_vert_patch();
