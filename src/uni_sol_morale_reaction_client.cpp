#include "uni_sol_morale_reaction_client.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(uni_sol_morale_reaction_client, 0x30);

uni_sol_morale_reaction_client::uni_sol_morale_reaction_client(universal_soldier_inode *a2,
                                                               universal_soldier_ability_server *a3)
{
    THISCALL(0x006AD790, this, a2, a3);
}
}  // namespace ai
