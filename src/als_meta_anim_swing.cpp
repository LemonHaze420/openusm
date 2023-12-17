#include "als_meta_anim_swing.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

namespace als {

VALIDATE_SIZE(als_meta_anim_swing, 0x70);

als_meta_anim_swing::als_meta_anim_swing()
{
    THISCALL(0x004AB870, this);
}

void als_meta_anim_swing::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("als::als_meta_anim_swing::unmash");

    a1->unmash_class_in_place(this->field_28, this);
}

float als_meta_anim_swing::get_anim_duration() const
{
    TRACE("als::als_meta_anim_swing::get_anim_duration");
    return 5.0;
}

}

void als_meta_anim_swing_patch()
{
    {
        FUNC_ADDRESS(address, &als::als_meta_anim_swing::get_anim_duration);
        SET_JUMP(0x00494210, address);
    }

    {
        FUNC_ADDRESS(address, &als::als_meta_anim_swing::_unmash);
        set_vfunc(0x0087B91C, address);
    }
}
