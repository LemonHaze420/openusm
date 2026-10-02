#include "ai_std_combat_target.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(ai::combat_target_inode, 0x88);

ai::combat_target_inode::combat_target_inode()
{
    if constexpr (STANDALONE_SYSTEM) {
        this->field_84 = false;
    } else {
        THISCALL(0x004406F0, this);
    }
}


void ai::player_web_target_inode::add_to_web_targets_list(vhandle_type<actor> a1)
{
    web_targets_list.push_back(a1);
}
