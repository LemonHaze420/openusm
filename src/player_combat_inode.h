#pragma once

#include "combat_inode.h"

namespace ai {

struct player_combat_inode : combat_inode {
    float field_328;
    int field_32C;

    player_combat_inode();
};

}  // namespace ai
