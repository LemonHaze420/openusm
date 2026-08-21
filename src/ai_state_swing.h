#pragma once

#include "info_node.h"

#include "enhanced_state.h"

#include "entity_base_vhandle.h"
#include "float.hpp"
#include "local_collision.h"
#include "mash.h"
#include "string_hash.h"
#include "vector3d.h"

struct entity_base;
struct region;

inline constexpr auto POINT_IS_VALID = 1u;
inline constexpr auto NORMAL_IS_VALID = 2u;
inline constexpr auto VISUAL_POINT_IS_VALID = 4u;
inline constexpr auto ENT_IS_VALID = 8u;
inline constexpr auto BEST_DISTANCE_SQUARED_IS_VALID = 0x10u;
inline constexpr auto TARGET_LENGTH_IS_VALID = 0x20u;

struct find_best_anchor_result_t {
    vector3d m_point;
    vector3d m_normal;
    vector3d m_visual_point;
    entity_base *m_entity;
    float m_best_distance_squared;
    float m_target_length;
    uint32_t valid_flags;

    //0x0048B200
    find_best_anchor_result_t();

    float get_target_length();

    void set_target_length(Float target_length);

    float get_best_distance_squared();

    void set_best_distance_squared(Float best_distance_squared);

    vector3d *get_point();

    void set_point(const vector3d &point);

    vector3d &get_normal();

    void set_normal(const vector3d &normal);

    vector3d &get_visual_point();

    void set_visual_point(const vector3d &a1);

    entity_base *get_entity();

    void set_entity(entity_base *a2);
};

struct from_mash_in_place_constructor;

namespace ai {

struct hero_inode;
struct ai_state_machine;
struct mashed_state;
struct param_block;

struct swing_state : enhanced_state {
    int field_30;
    int field_34;
    int field_38;
    hero_inode *field_3C;

    //0x0045AC40
    swing_state();

    //0x0045AC60
    swing_state(from_mash_in_place_constructor *a2);

    //virtual
    ~swing_state();

    void finalize(mash::allocation_scope a2);

    //0x0044BAA0
    //virtual
    uint32_t get_virtual_type_enum();

    //0x0047DA60
    //virtual
    void _activate(ai_state_machine *a2, const mashed_state *arg4, const mashed_state *a4, const param_block *a5,
                   base_state::activate_flag_e a6);

    //0x0044BB00
    //virtual
    void _deactivate(const ai::mashed_state *a1);

    //0x0047DDD0
    /* virtual */ state_trans_messages _frame_advance(Float a2);

    /* virtual */ void get_info_node_list(info_node_desc_list &a1);

    /* virtual */ int get_mash_sizeof();

    static const inline string_hash default_id{to_hash("SWING")};

    static const inline string_hash auto_crawl_attach_id{to_hash("SWING_AUTO_CRAWL_ATTACH")};
};

struct swing_inode : info_node {
    bool field_1C;
    hero_inode *field_20;
    bool field_24;
    float m_swing_time;
    float field_2C;
    float m_constraint;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    bool field_44;
    vector3d field_48;
    bool field_54;
    vector3d field_58;
    vector3d field_64;
    vector3d field_70;
    float field_7C;
    float m_anchor_angle;
    float field_84;
    float field_88;
    float field_8C;
    float field_90;

    //0x00481560
    swing_inode();

    //0x004882C0
    bool is_eligible(string_hash a2, Float a3);

    //0x0045C7B0
    void play_fire_web_sound();

    //0x0044C1F0
    float get_swing_cur_max_anchor_dist() const;

    //0x0045AF30
    void clip_web(Float a2);

    //0x0044C320
    void cleanup_swingers();

    //0x00478820
    void fire_new_web(bool is_play_fire_web_sound);

    //0x0044BEB0
    void compute_forward_and_up_directions(vector3d &forward, vector3d &up);

    //0x0044BB80
    vector3d get_desired_up_facing() const;

    //0x0044C280
    float compute_ground_level_at_sweet_spot_position(const vector3d &a1, Float a2, const region *reg);

    //0x0046AB90
    void compute_dynamic_sweet_spot_params(float *result_angle, float *result_length, float *result_ground_level,
                                           vector3d *result_direction);

    //0x004783B0
    void setup_new_web();

    //0x004779B0
    void update_mode_swinging(Float a2);

    //0x0045C850
    bool can_go_to(string_hash a2) const;

    //0x0045AD80
    void update_something_to_swing_to(Float a2);

    //0x00488060
    void find_best_anchor_point(const vector3d &a1, const vector3d &a2, const vector3d &a3, const entity_base *a4,
                                Float length, float *sweet_spot_ground_level);

    //0x0044C100
    void propose_something_to_swing_to(vector3d a1, entity_base *a4, vector3d a5, vector3d normal, Float a11, Float a12,
                                       vector3d visual_point);

    //0x004875F0
    void init_swingers();

    //0x0045B110
    void update_pendulums(Float a2);

    //0x0045C3C0
    void check_for_collision();

    //0x0046B180
    static void do_web_splat(vector3d target_point, vector3d target_normal,
                             const local_collision::entfilter_base &entfilter_arg);

    //0x00488040
    //virtual
    void frame_advance(Float a2);

    //virtual
    void _activate(ai::ai_core *a2);

    //virtual
    void _deactivate();

    static const inline string_hash default_id{to_hash("SWING")};
};

}  // namespace ai

struct something_to_swing_to_data_t {
    float field_0;
    float field_4;
    entity_base_vhandle field_8;
    vector3d field_C;
    vector3d field_18;
    float goal_constraint;
    vector3d field_28;
    int field_34;
    vector3d field_38;
    vector3d m_normal;
    vector3d m_visual_point;
    float m_best_distance;
    float m_target_length;
    bool field_64;
};

extern void swing_inode_patch();

extern void swing_state_patch();
