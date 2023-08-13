#pragma once

#include "info_node.h"

#include "corner_info.h"
#include "crawl_request_type.h"
#include "float.hpp"
#include "vector3d.h"

struct from_mash_in_place_constructor;

namespace local_collision {
struct primitive_list_t;
}

namespace ai {

struct physics_inode;
struct als_inode;
struct corner_info;
struct swing_inode;
struct web_zip_inode;
struct spidey_combat_inode;
struct controller_inode;
struct base_full_target_inode;
struct glass_house_inode;
struct interaction_inode;
struct pole_swing_inode;

struct hero_inode : info_node {
    struct internal {
        int field_0;
        int field_4[7];
        line_info field_20;
        corner_info field_7C;
    };

    bool field_1C;
    als_inode *field_20;
    controller_inode *field_24;
    physics_inode *field_28;
    spidey_combat_inode *field_2C;
    base_full_target_inode *field_30;
    interaction_inode *field_34;
    int field_38;
    web_zip_inode *field_3C;
    swing_inode *field_40;
    glass_house_inode *field_44;
    pole_swing_inode *field_48;
    char field_4C;
    int field_50;

    int field_54;
    vector3d field_58;
    vector3d field_64;

    float field_70;
    float field_74;
    int field_78[2];

    local_collision::primitive_list_t *field_80;
    local_collision::primitive_list_t *field_84;

    ai::hero_inode::internal field_88;

    int field_1AC;

    line_info field_1B0;
    int field_20C[11];

    float field_238;
    entity_base_vhandle field_23C;
    int field_240;
    float field_244;
    int field_248;

    //0x006A1B10
    hero_inode(from_mash_in_place_constructor *a2);

    void sub_68A7F0(int a2, bool a3);

    //0x006942A0
    void cleanup_collision_lists();

    //0x006A7110
    bool jump_is_eligible(string_hash a2);

    //0x006A6340
    bool crawl_can_go_to(string_hash a2, string_hash a3);

    //0x006944D0
    int engage_water_exit();

    //0x006A76D0
    bool run_can_go_to(string_hash arg0);

    //0x006A6280
    bool ought_to_jump_off_wall(line_info &a2);

    //0x0069FAF0
    bool ought_to_stick_to_wall(line_info &a2, bool a3);

    //0x00693B00
    bool accept_crawl_spot(vector3d a1, vector3d a4);

    //0x0068A060
    static bool is_a_crawl_state(string_hash a1, bool a2);

    //0x006A5CF0
    bool get_closest_corner(corner_info *a2, crawl_request_type a3);

    //0x006A7770
    bool run_is_eligible(string_hash a2);

    //0x006B0EB0
    bool crawl_is_eligible(string_hash a2, bool a3);

    //0x006B0E60
    bool oldcrawl_is_eligible(string_hash a2, bool a3);

    //0x006A6900
    bool crawl_is_eligible_internals(string_hash a2, bool a3);

    //0x006A67E0
    void update_wall_run_als_params();

    //0x006A6E70
    bool jump_can_go_to(string_hash a2);

    //0x006A7950
    //virtual
    void frame_advance(Float a2);

    static int get_hero_type();

    static const inline string_hash default_id{to_hash("hero")};
};

//0x0068A5F0
extern void shrink_capsule_for_slanted_surfaces(actor *act);

extern void extend_capsule_for_jump(actor *act);

} // namespace ai

//0x0069F9A0
extern bool have_relative_movement(entity *a1, entity *a2);

extern void hero_inode_patch();
