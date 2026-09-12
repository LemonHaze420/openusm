#include "player_combat_inode.h"

#include "common.h"
#include "script_manager.h"

namespace ai {

VALIDATE_SIZE(player_combat_inode, 0x330);

player_combat_inode::player_combat_inode() {}

void player_combat_inode::_activate(ai_core *a2)
{
    combat_inode::_activate(a2);
    this->field_328 = 0.0f;
    mString v1{"gv_combat_level"};
    this->field_32C = static_cast<float *>(script_manager::get_game_var_address(v1, nullptr, nullptr));
}

}  // namespace ai
