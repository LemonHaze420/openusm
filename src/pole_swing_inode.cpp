#include "pole_swing_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(pole_swing_inode, 0x2C);

pole_swing_inode::pole_swing_inode() {}

bool pole_swing_inode::can_go_to(string_hash arg0)
{
    return THISCALL(0x0045D190, this, arg0);
}

}  // namespace ai
