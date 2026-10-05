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

        nalInstance *_scalar_deleting_destructor(uint32_t flags);
        void _get_pose(Float t, Float t_prev, nalBasePose &pose, const nalBasePose &default_pose);
        void _blend_two_anims(Float t0, Float t1, nalAnyPose &pose, const nalAnyPose &default_pose, Float blend,
                              nalInstanceClass *anim0, nalInstanceClass *anim1);

        static inline void *g_vtbl[] = {
            func_address(&nalInstance::_scalar_deleting_destructor),
            func_address(&nalInstance::_get_pose),
            func_address(&nalInstance::_blend_two_anims),
        };
    };

    mVector<meta_key_anim> field_28;
    nalAnyPose *field_3C[12];
    int field_6C;

    als_meta_anim_swing();

    als_meta_anim_swing(from_mash_in_place_constructor *);

    ~als_meta_anim_swing();

    static void *native_vtable();

    void _destruct_mashed_class();
    als_meta_anim_swing *_scalar_deleting_destructor(uint32_t flags);
    bool _is_subclass_of(mash::virtual_types_enum type) const;
    bool _is_pivot_valid() const;
    void _release(void *);

    void finalize();

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
        func_address(&als_meta_anim_swing::_destruct_mashed_class),
        func_address(&als_meta_anim_swing::_unmash),
        func_address(&als_meta_anim_swing::_scalar_deleting_destructor),
        func_address(&als_meta_anim_swing::_get_virtual_type_enum),
        func_address(&als_meta_anim_swing::_is_subclass_of),
        func_address(&mash_virtual_base::_is_or_is_subclass_of),
        func_address(&als_meta_anim_base::_get_anim_name),
        func_address(&als_meta_anim_swing::_is_anim_looping),
        func_address(&als_meta_anim_swing::_is_anim_trajectory_relative),
        func_address(&als_meta_anim_swing::_get_anim_duration),
        func_address(&als_meta_anim_swing::_get_skeleton),
        func_address(&als_meta_anim_swing::_create_anim_inst),
        func_address(&als_meta_anim_swing::_is_pivot_valid),
        func_address(&als_meta_anim_swing::_release),
        func_address(&als_meta_anim_swing::_get_mash_sizeof),
    };
};

}  // namespace als

extern void als_meta_anim_swing_patch();
