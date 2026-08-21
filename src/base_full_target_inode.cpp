#include "base_full_target_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {
VALIDATE_SIZE(base_full_target_inode, 0x84);

base_full_target_inode::base_full_target_inode(from_mash_in_place_constructor *a2)
{
    THISCALL(0x006A1990, this, a2);
}

vhandle_type<actor> base_full_target_inode::quick_targeting()
{
    return this->field_34;
}

bool base_full_target_inode::is_target_known()
{
    return this->field_38 <= 5;
}

}  // namespace ai
