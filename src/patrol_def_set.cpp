#include "patrol_def_set.h"

#include "common.h"
#include "func_wrapper.h"
#include "patrol_def.h"

VALIDATE_SIZE(patrol_def_set, 0x14u);

patrol_def_set::patrol_def_set(from_mash_in_place_constructor *a2)
{
    if constexpr (0)
    {}
    else
    {
        THISCALL(0x005DD180, this, a2);
    }
}

