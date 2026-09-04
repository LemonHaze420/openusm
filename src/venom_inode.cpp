#include "venom_inode.h"

#include "common.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"

VALIDATE_SIZE(venom_inode, 0xBC);

venom_inode::venom_inode(from_mash_in_place_constructor *a2)
{
    THISCALL(0x0072F650, this, a2);
}
