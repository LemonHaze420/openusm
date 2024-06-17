#pragma once

#include "base_state.h"
#include "enhanced_state.h"
#include "float.hpp"
#include "mash_virtual_base.h"
#include "string_hash.h"
#include "vector3d.h"

struct from_mash_in_place_constructor;

namespace ai {

struct hero_inode;
struct ai_state_machine;
struct mashed_state;
struct param_block;
struct info_node_desc_list;

struct jump_param_t {
    jump_param_t(const char *a1, const char *a2)
        : m_height(a1), m_distance(a2), field_8(), field_C() {}

    string_hash m_height;
    string_hash m_distance;
    string_hash field_8;
    string_hash field_C;
};

struct jump_state : enhanced_state {
    hero_inode *field_30;
    vector3d field_34;
    vector3d field_40;
    vector3d field_4C;
    float field_58;
    float field_5C;
    float field_60;
    vector3d field_64;
    float field_70;
    float field_74;
    float field_78;
    int field_7C;

    bool field_80;
    bool field_81;

    int field_84[8];

    //0x00449D10
    jump_state(from_mash_in_place_constructor *a2);

    void apply_jets(Float a1);

    //0x00469EF0
    bool process_flying(Float a2);

    //0x004599C0
    void initiate();

    //0x0044A150
    bool check_for_dive_fall();

    //0x0044A770
    void initiate_from_wall();

    //0x0044AB30
    void initiate_from_ground();

    //0x0044AD40
    void initiate_super_jump();

    //0x0044ADA0
    void initiate_from_swing();

    //0x0044B120
    void initiate_from_pole_swing();

    //0x0044B220
    void initiate_from_air();

    //0x0044A640
    vector3d compute_force(vector3d a3, vector3d a4) const;

    //0x0044A230
    void set_gravity_vector(const vector3d &a2, Float a3);

    //0x0044A580
    vector3d calculate_jump_vector(vector3d a3, vector3d a6, Float a9, Float a10) const;

    //0x00469AC0
    //virtual
    int frame_advance_jump_type_specifics(Float a2);

    //0x00473E70
    /* virtual */ int frame_advance(Float a2);

    //0x0044A3B0
    //virtual
    int get_info_node_list(info_node_desc_list &a1);

    //0x00438C50
    //virtual
    int get_mash_sizeof();

    //0x0043BAE0
    //virtual
    bool is_subclass_of(mash::virtual_types_enum a2);

    //0x00469880
    //virtual
    int activate(ai_state_machine *a2,
                 const mashed_state *a3,
                 const mashed_state *a4,
                 const param_block *a5,
                 base_state::activate_flag_e a6);

    //0x00449FA0
    //virtual
    void deactivate(const ai::mashed_state *a1);

    //0x00449D50
    /* virtual */ uint32_t get_virtual_type_enum();

    static const inline string_hash default_id{to_hash("jump")};
};
} // namespace ai

extern void jump_state_patch();
