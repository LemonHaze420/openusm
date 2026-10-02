#pragma once

#include "info_node.h"

#include "combo_system_move.h"

struct ai_tentacle_info;

namespace ai {

struct als_inode;
struct combat_target_inode;
struct damage_inode;
struct controller_inode;

struct combat_inode : info_node {
    struct incoming_move {
        std::intptr_t m_vtbl;
        int field_4;
        int field_8;
        int field_C;
        int field_10;
        combo_system_move::results field_14;
        bool field_90;

        //0x00467020
        incoming_move();

        //0x0048DB20
        incoming_move(const incoming_move &a2);
    };

    int field_1C;
    int field_20;
    controller_inode *field_24;
    als_inode *field_28;
    combat_target_inode *field_2C;
    damage_inode *field_30;
    ai_tentacle_info *field_34;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
    int field_54;
    float field_58;
    float field_5C;
    float field_60;
    int field_64;
    int field_68;
    int field_6C;
    int field_70;
    int field_74;
    int field_78;
    int field_7C;
    bool field_80;
    bool field_81;
    bool field_82;
    bool field_83;
    bool field_84;
    bool field_85;
    bool field_86;
    int field_88;
    int field_8C;
    int field_90;
    int field_94;
    int field_98;
    int field_9C;
    int field_A0;
    int field_A4;
    int field_A8;
    int field_AC;
    int field_B0;
    bool field_B4;

    int field_[156];

    combat_inode();

    //0x00467440
    void _activate(ai_core *a2);

    //virtual
    combo_system_move *get_cur_move();

    //virtual
    bool needs_hit_react(Float a2);

    //0x00454430
    void _clear_cur_move();

    //virtual
    void clear_cur_move();

    //virtual
    bool has_next_move();

    //0x00454490
    void _clear_next_move();

    //virtual
    void clear_next_move();

    //0x0043FC90
    //virtual
    void left_air();

    static inline string_hash default_id{to_hash("combat_inode")};

    static inline string_hash reject_all_hash{int(to_hash("reject_all_attacks"))};
    static inline string_hash always_keep_target_hash{int(to_hash("always_keep_target"))};
};

}  // namespace ai
