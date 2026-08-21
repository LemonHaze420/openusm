#pragma once

#include "als_layer_types.h"
#include "fixedstring.h"
#include "float.hpp"
#include "vector3d.h"

#include <cstdint>

struct actor;
struct nalBaseSkeleton;
struct po;
struct string_hash;

namespace als {
struct als_meta_anim_table_shared;
}

struct nalAnyPose;

template <typename T>
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

        float get_anim_time_in_sec() const;

        float get_anim_speed() const;
    };

    std::intptr_t m_vtbl;
    actor *field_4;
    nalBaseSkeleton *field_8;
    const als::als_meta_anim_table_shared *field_C;
    unsigned int field_10;

    //0x00492530
    animation_controller(actor *a2, nalBaseSkeleton *a3, unsigned int a4, const als::als_meta_anim_table_shared *a5);

    ~animation_controller();

    void get_camera_root_abs_po(po &arg0);

    bool is_same_animtype(tlFixedString a2) const;

    anim_ctrl_handle get_base_anim_handle();

    anim_ctrl_handle play_layer_anim(const string_hash &a3, unsigned int a4, Float a5, unsigned int a6, bool a7,
                                     als::layer_types);

    anim_ctrl_handle *_play_base_layer_anim_patch(anim_ctrl_handle *, const string_hash &a3, Float a4, uint32_t a5,
                                                  bool a6);

    anim_ctrl_handle play_base_layer_anim(const string_hash &a3, Float a4, uint32_t a5, bool a6);

    //virtual
    void play_layer_anim(nalAnimClass<nalAnyPose> *, Float, Float, uint32_t, bool, void *) /* = 0 */;

    //virtual
    void play_base_layer_anim(nalAnimClass<nalAnyPose> *a2, Float a3, Float a4, bool a5, bool a6, void *a7) /* = 0 */;

    //virtual
    void kill_domain(uint32_t, Float) /* = 0 */;

    //virtual
    void kill_specific_anim(Float, Float) /* = 0 */;

    //virtual
    void begin_scene_animation() /* = 0 */;

    //virtual
    void end_scene_animation() /* = 0 */;

    //virtual
    void scene_animation_playing() /* = 0 */;

    //virtual
    bool is_anim_active(Float a1) const;

    //virtual
    float get_base_anim_time_in_sec() const;

    //virtual
    float get_anim_time_in_sec(Float a2) const;

    //virtual
    float get_base_anim_speed();

    //virtual
    float get_anim_speed(Float a2);

    //virtual
    void set_base_anim_speed(Float);

    //virtual
    void reset();

    //virtual
    void *get_base_layer_anim_ptr();

    //virtual
    void *get_anim_ptr(Float);

    //virtual
    void offscreen_frame_advance(Float);

    //virtual
    void frame_advance(Float a2, bool a3, bool a4);

    //virtual
    void get_curr_po_offset(po &);

    //virtual
    float get_floor_offset();

    //virtual
    float get_camera_fov();

    //virtual
    float get_camera_far_clip();

    //virtual
    float get_tentacle_width(string_hash);

    //virtual
    float get_tentacle_activity(string_hash);

    //virtual
    float get_tentacle_pull_factor(string_hash);

    //virtual
    void get_camera_root_rel_po(po &);

    //virtual
    void get_shake_root_rel_po(po &a3);

    //virtual
    bool will_have_hint_token_scale(string_hash);

    //virtual
    vector3d get_hint_token_scale(string_hash);
};

extern void animation_controller_patch();
