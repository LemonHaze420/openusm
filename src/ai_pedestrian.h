#pragma once

#include "actor.h"
#include "info_node.h"
#include "enhanced_state.h"
#include "entity_base_vhandle.h"
#include "float.hpp"

struct ped_spawner;
struct traffic_path_lane;

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
    char field_64[0x2C];
    int field_90;
    int field_94;
    int field_98;
    vector3d field_9C;
    int field_A8;
    int field_AC;
    int field_B0;
    float m_hit_points;
    int field_B8;
    int field_BC;
    int field_C0;
    float field_C4;
    int field_C8;
    int field_CC;
    bool m_ped_spawner_cleared;
    bool field_D1;
    int field_D4;

    pedestrian_inode();

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

    static void unregister_non_ped(vhandle_type<actor> a1);

    static inline const string_hash default_id{int(to_hash("PEDESTRIAN"))};

    static inline const string_hash elevation_adj_hash{int(to_hash("elevation_adj"))};

    static inline const string_hash hit_points_hash{int(to_hash("hit_points"))};

    static inline auto &timer = var<Float>(0x0096C114);
};

struct pedestrian_idle_state : enhanced_state {
    void pedize_non_pedestrian();
};

}  // namespace ai
