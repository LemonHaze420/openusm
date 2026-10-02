#include "spidey_combat_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(spidey_combat_inode, 0x350);

spidey_combat_inode::spidey_combat_inode() {}

void spidey_combat_inode::_activate(ai_core *a2)
{
    TRACE("spidey_combat_inode::activate");

    player_combat_inode::_activate(a2);
    this->field_330 = -1.0;
    this->field_334 = 0;
    this->field_338 = 0;
    this->field_348 = nullptr;
    this->field_34C = nullptr;
}

void spidey_combat_inode::update_pending_move(combat_inode::incoming_move a2)
{
    THISCALL(0x0069B810, this, a2);
}

}  // namespace ai

void spidey_combat_inode_patch()
{
    {
        FUNC_ADDRESS(address, &ai::spidey_combat_inode::_activate);
        set_vfunc(0x0087D720, address);
    }
}
