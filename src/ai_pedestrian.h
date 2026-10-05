#pragma once

#include "actor.h"
#include "info_node.h"
#include "enhanced_state.h"
#include "entity_base_vhandle.h"
#include "float.hpp"
#include <list.hpp>

struct ped_spawner;
struct traffic_path_lane;
struct point_of_interest;

namespace ai {

struct ai_core;
struct als_inode;

struct pedestrian_inode : info_node {
    uint32_t field_1C;
    als_inode *field_20;
    ped_spawner *m_ped_spawner;
    float m_elevation_adj;
    int field_2C;
    vector3d field_30;
    vector3d field_3C;
    traffic_path_lane *field_48;
    vector3d field_4C;
    vector3d field_58;
    float dodge_distance_squared;
    vector3d dodge_position;
    vector3d dodge_velocity_direction;
    vector3d dodge_direction;
    int field_8C;
    int field_90;
    float field_94;
    float field_98;
    vector3d field_9C;
    int field_A8;
    point_of_interest *field_AC;
    float field_B0;
    float m_hit_points;
    float field_B8;
    float field_BC;
    float field_C0;
    float field_C4;
    float field_C8;
    int field_CC;
    bool m_ped_spawner_cleared;
    bool field_D1;
    int field_D4;

    pedestrian_inode();
    explicit pedestrian_inode(from_mash_in_place_constructor *constructor);
    static void *native_vtable();
    bool does_need_advance() const;
    void frame_advance(Float elapsed);
    void reset();

    vector3d get_desired_dir() const;
    bool is_on_right_side_of_road();
    float get_lane_fraction(const vector3d &direction) const;
    void update_lane_confinement_als_params();
    void update_cur_poi();
    void update_est_speed();
    void update_screaming();
    void update_hidden_time();
    bool should_show() const;
    void update_elevation();
    void calc_elevation(float &elevation, float &ground, const vector3d &position, bool limit_rise);
    void update_lane_endpoints(traffic_path_lane *lane);
    vector3d get_adjusted_lane_node(traffic_path_lane *lane, int index);
    int get_nearest_adjusted_node_index(traffic_path_lane *lane, const vector3d &position);
    bool get_next_and_prev_lane_node(vector3d &next, vector3d &previous);
    void update_dodging();
    void check_dodge(entity_base *other, const vector3d &direction, float distance_squared, float speed);
    void update_dodge_direction();
    void update_greeting();

    bool is_flagged(uint32_t a2) const
    {
        return (a2 & this->field_1C) != 0;
    }

    void initialize(mash::allocation_scope a2);

    void set_ped_spawner(ped_spawner *a2);

    traffic_path_lane *get_cur_lane() const;

    void set_flag(uint32_t a3, bool a4);

    void set_can_dodge(int a1);

    void restore_hit_pts();

    void sub_696AF0(Float a2);

    //virtual
    //0x006ADB90
    void activate(ai_core *a2);

    static bool is_a_pedestrian(ai::ai_core *a1);

    //0x006A1260
    static void register_non_ped(vhandle_type<actor> a3);

    static void unregister_non_ped(vhandle_type<actor> a1);

    static inline const string_hash default_id{int(to_hash("PEDESTRIAN"))};

    static inline const string_hash elevation_adj_hash{int(to_hash("elevation_adj"))};

    static inline const string_hash hit_points_hash{int(to_hash("hit_points"))};

#if STANDALONE_SYSTEM
    static inline uint32_t timer{};
    static inline int general_last_elev_update_frame{-1};
    static inline float next_greeting_time{};
    static inline _std::list<vhandle_type<actor>> *non_ped_list{};
#else
    static inline auto &timer = var<uint32_t>(0x0096C114);
    static inline auto &general_last_elev_update_frame = var<int>(0x0096BE6C);
    static inline auto &next_greeting_time = var<float>(0x0096C858);
    static inline auto *&non_ped_list = var<_std::list<vhandle_type<actor>> *>(0x0096BE74);
#endif
};

struct pedestrian_idle_state : enhanced_state {
    pedestrian_idle_state();
    explicit pedestrian_idle_state(from_mash_in_place_constructor *constructor);
    static void *native_vtable();
    void activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                  const param_block *parameters, activate_flag_e flags);
    state_trans_messages frame_advance(Float elapsed);
    void pedize_non_pedestrian();
};

}  // namespace ai
