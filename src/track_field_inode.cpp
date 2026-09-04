#include "track_field_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(track_field_inode, 0x3C);

ai::track_field_inode::track_field_inode()
{
    THISCALL(0x00443130, this);
}

track_field_inode::track_field_inode(from_mash_in_place_constructor *a2)
{
    THISCALL(0x006A2280, this, a2);
}

}  // namespace ai
