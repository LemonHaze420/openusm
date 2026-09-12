#pragma once

#include "combat_inode.h"

namespace ai {

struct player_combat_inode : combat_inode {
    float field_328;
    float *field_32C;

    player_combat_inode();

    //0x00467B90
    void _activate(ai_core *a2);
};

}  // namespace ai
