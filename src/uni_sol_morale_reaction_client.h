#pragma once

#include <cstdint>

#include "param_block.h"

namespace ai {

struct universal_soldier_ability_server;
struct universal_soldier_inode;

struct uni_sol_morale_reaction_client {
    std::intptr_t m_vtbl;
    universal_soldier_ability_server *field_4;
    bool field_8;
    bool field_9;
    universal_soldier_inode *field_C;
    float field_10;
    float field_14;
    float field_18;
    float field_1C;
    bool field_20;
    bool field_21;
    param_block field_24;

    //0x006AD790
    uni_sol_morale_reaction_client(universal_soldier_inode *a2, universal_soldier_ability_server *a3);
};
}  // namespace ai
