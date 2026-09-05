#include "ai_std_combat_target.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

ai::combat_target_inode::combat_target_inode()
{
    THISCALL(0x004406F0, this);
}

void ai::player_web_target_inode::add_to_web_targets_list(vhandle_type<actor> a1)
{
    web_targets_list.push_back(a1);
}
