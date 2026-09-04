#include "spidey_combat_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(spidey_combat_inode, 0x350);

spidey_combat_inode::spidey_combat_inode() {}

void spidey_combat_inode::update_pending_move(combat_inode::incoming_move a2)
{
    THISCALL(0x0069B810, this, a2);
}

}  // namespace ai
