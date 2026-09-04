#include "scene_brew.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(scene_brew, 0xD4);

scene_brew::scene_brew(const resource_key &a2, limited_timer *a3)
{
    THISCALL(0x005678E0, this, &a2, a3);
}
