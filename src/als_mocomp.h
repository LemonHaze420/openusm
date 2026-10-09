#pragma once

#include "als_motion_compensator.h"
#include "string_hash.h"

struct po;

namespace als {

struct animation_logic_system;

struct begin_biped_physics : motion_compensator {
    bool field_14;
    int field_18;
    float field_1C;

    //virtual
    float activate(animation_logic_system *);

    static inline string_hash velocity_left_c_param_hash{int(to_hash("velocity_left_c"))};

    static inline string_hash velocity_y_left_c_param_hash{int(to_hash("velocity_y_left_c"))};

    static inline string_hash velocity_left_deactivate_c_param_hash{int(to_hash("velocity_left_deactivate_c"))};

    static inline string_hash force_scale_param_hash{int(to_hash("force_scale"))};

    static inline string_hash force_y_param_hash{int(to_hash("force_y"))};

    static inline string_hash rotate_xz_ang_param_hash{int(to_hash("rotate_xz_ang"))};
};

struct move_and_face_no_anim_movement : motion_compensator {
    vector3d initial_position;
    vector3d destination;
    vector3d facing;
    vector3d up;
    float movement_speed;
    float turn_rate;
    int translation_mode;
    int orientation_mode;
    int expired_translation_mode;
    int expired_orientation_mode;
    int translation_disabled;
    int orientation_disabled;
    float remaining_time;
    string_hash *destination_id;
    int field_6C;
    bool field_70;
    char field_71[3];

    void activate(animation_logic_system *);
    void post_anim_action(Float);
    vector3d simple_linear_translation(Float, float fraction, const po &);
    po simple_linear_orientation(Float, float fraction, const po &);
    vector3d compute_translation(Float, const po &, const po &);
    po compute_orientation(Float, const po &, const po &);
    vector3d select_translation(Float, const po &, const po &);
    po select_orientation(Float, const po &, const po &);
    po apply_animation_offset();
    void face_and_arrive_by(Float);
    void destruct_mashed_class();
    void unmash(mash_info_struct *, void *);
};
struct combat_move_and_face : move_and_face_no_anim_movement {
    void activate(animation_logic_system *);
    static void *native_vtable();
};

struct feed_mocomp : move_and_face_no_anim_movement {
    void activate(animation_logic_system *);
    static void *native_vtable();
};

struct constant_move_and_face : move_and_face_no_anim_movement {
    void activate(animation_logic_system *);
};

struct crawl_transition : constant_move_and_face {
    void activate(animation_logic_system *);
    static void *native_vtable();
};

struct crawl_orient : motion_compensator {
    void post_anim_action(Float);
    static void *native_vtable();
};

struct move_and_face : motion_compensator {
    vector3d animation_target_offset;
    bool grounded;
    char pad_21[3];
    vector3d velocity;
    vector3d acceleration;
    vector3d added_velocity;
    bool has_target;
    bool target_initialized;
    char pad_4A[2];
    vector3d target;
    vector3d initial_target;
    float target_cheat_k;
    bool reverse_facing;
    char pad_69[3];
    float land_max_slowdown_rate;
    bool saved_collision;
    bool saved_gravity;
    bool saved_motion;
    char pad_73;

    void activate(animation_logic_system *);
    void deactivate();
    void post_anim_action(Float);
    static void *native_vtable();
};

struct flight_mocomp : motion_compensator {
    vector3d destination;
    vector3d target_offset;
    vector3d target_direction;
    vector3d target_facing;
    float target_cheat_k;
    float restore_visible_timer;
    bool restored_visible;
    bool saved_collision;
    bool saved_gravity;
    bool saved_motion;

    void activate(animation_logic_system *);
    void deactivate();
    void post_anim_action(Float);
    static void *native_vtable();
};

}  // namespace als

extern void als_mocomp_patch();
