#pragma once

#include "info_node.h"

#include <list.hpp>
#include <vector.hpp>

struct from_mash_in_place_constructor;

namespace ai {

struct ai_car_inode;
struct ai_state_machine;
struct std_fear_inode;
struct universal_soldier_ability_client;
struct universal_soldier_attack_token;

struct universal_soldier_inode : info_node {
    string_hash field_1C;
    universal_soldier_attack_token *field_20;
    _std::list<universal_soldier_attack_token *> *field_24;
    ai_state_machine *field_28;
    int field_2C;
    universal_soldier_inode *field_30;
    ai_car_inode *field_34;
    int field_38;
    int field_3C;
    std_fear_inode *field_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
    _std::vector<universal_soldier_ability_client *> *field_54;
    int field_58;
    int field_5C;
    int field_60;
    int field_64;
    int field_68;
    int field_6C;
    bool field_70;
    float field_74;

    //0x006B58F0
    universal_soldier_inode(from_mash_in_place_constructor *a2);

    static vector3d get_combat_spacing_pos(int slot, const vector3d &a3, Float a4);
};
}  // namespace ai
