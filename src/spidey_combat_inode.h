#pragma once

#include "player_combat_inode.h"

namespace ai {

struct spidey_combat_inode : player_combat_inode {
    int field_330[8];

    spidey_combat_inode();

    //0x0069B810
    //virtual
    void update_pending_move(combat_inode::incoming_move a2);
};
}  // namespace ai
