#include "als_simple_orientation.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

namespace als {

void simple_orientation::post_anim_action(Float a2)
{
    TRACE("als::simple_orientation::post_anim_action");

    THISCALL(0x004A13B0, this, a2);
}

}  // namespace als

void als_simple_orientation_patch()
{
    FUNC_ADDRESS(address, &als::simple_orientation::post_anim_action);
    set_vfunc(0x008785F4, address);
}
