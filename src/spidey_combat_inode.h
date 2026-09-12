#pragma once

#include "player_combat_inode.h"

struct entity;

namespace ai {

struct spidey_combat_inode : player_combat_inode {
    float field_330;
    int field_334;
    int field_338;
    int field_33C;
    int field_340;
    int field_344;
    int *field_348;
    entity **field_34C;

    spidey_combat_inode();

    //0x00697420
    //virtual
    void _activate(ai_core *a2);

    //0x0069B810
    //virtual
    void update_pending_move(combat_inode::incoming_move a2);
};
}  // namespace ai

extern void spidey_combat_inode_patch();
