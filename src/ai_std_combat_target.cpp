#include "ai_std_combat_target.h"

void ai::player_web_target_inode::add_to_web_targets_list(vhandle_type<actor> a1)
{
    web_targets_list.push_back(a1);
}
