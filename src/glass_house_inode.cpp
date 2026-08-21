#include "glass_house_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(glass_house_inode, 0x34);

glass_house_inode::glass_house_inode() {}

void glass_house_inode::show_glass_house_message()
{
    THISCALL(0x00455F00, this);
}

}  // namespace ai
