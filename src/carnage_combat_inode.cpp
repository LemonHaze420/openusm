#include "carnage_combat_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(carnage_combat_inode, 0x36C);

carnage_combat_inode::carnage_combat_inode(from_mash_in_place_constructor *a2)
{
    THISCALL(0x0072F090, this, a2);
}

void carnage_combat_inode::frame_advance(Float a2)
{
    THISCALL(0x0071FDC0, this, a2);
}

}  // namespace ai
