#pragma once

#include "info_node.h"

#include "combo_system_move.h"

struct ai_tentacle_info;

namespace ai {

struct als_inode;

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
    int field_24;
    als_inode *field_28;
    int *field_2C;
    int field_30;
    ai_tentacle_info *field_34;
    int field_38;
    int field_3C[11];
    int field_68;
    int field_6C;

    int field_[174];

    combat_inode();

    //virtual
    combo_system_move *get_cur_move();

    //virtual
    bool needs_hit_react(Float a2);

    //virtual
    bool has_next_move();

    //0x0043FC90
    //virtual
    void left_air();
};

}  // namespace ai
