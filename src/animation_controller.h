#pragma once

#include "als_layer_types.h"
#include "fixedstring.h"
#include "float.hpp"

#include <cstdint>

struct actor;
struct nalBaseSkeleton;
struct po;
struct string_hash;

namespace als {
struct als_meta_anim_table_shared;
}

struct nalAnyPose;

template<typename T>
struct nalAnimClass;

struct animation_controller {

    struct anim_ctrl_handle {
        bool field_0;
        float field_4;
        animation_controller *field_8;

        bool is_same_animtype(tlFixedString a1) const;

        void set_anim_speed(Float a2);

        bool is_anim_active() const;

        void *get_anim_ptr() const;

        float get_anim_norm_time() const;

        double get_anim_time_in_sec() const;

        double get_anim_speed() const;
    };

    std::intptr_t m_vtbl;
    actor *field_4;
    nalBaseSkeleton *field_8;
    als::als_meta_anim_table_shared *field_C;

    void get_camera_root_abs_po(po &arg0);

    bool is_same_animtype(tlFixedString a2) const;

    anim_ctrl_handle get_base_anim_handle();

    anim_ctrl_handle play_layer_anim(
            const string_hash &a3,
            unsigned int a4,
            Float a5,
            unsigned int a6,
            bool a7,
            als::layer_types);

    anim_ctrl_handle *_play_base_layer_anim(
        anim_ctrl_handle *,
        const string_hash &a3,
        Float a4,
        uint32_t a5,
        bool a6);

    anim_ctrl_handle play_base_layer_anim(
        const string_hash &a3,
        Float a4,
        uint32_t a5,
        bool a6);

    //virtual
    void reset();

    //virtual
    void play_base_layer_anim(
        nalAnimClass<nalAnyPose> *a2,
        Float a3,
        Float a4,
        bool a5,
        bool a6,
        void *a7);

    //virtual
    bool is_anim_active(Float a1) const;

    //virtual
    double get_base_anim_time_in_sec() const;

    //virtual
    double get_anim_time_in_sec(Float a2) const;

    //virtual
    double get_base_anim_speed();

    //virtual
    double get_anim_speed(Float a2);

    //virtual
    void frame_advance(Float a2, bool a3, bool a4);
};

extern void set_black_suit_als_meta_anim_table(
    const als::als_meta_anim_table_shared *table);

//0x0049B910
extern void *get_anim_by_hash(
        const string_hash &a1,
        const als::als_meta_anim_table_shared *a2,
        actor *a3);

extern void animation_controller_patch();
extern void venom_animation_lookup_patch();
