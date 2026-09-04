#include "patrol_def_set.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "patrol_def.h"

VALIDATE_SIZE(patrol_def_set, 0x14u);

patrol_def_set::patrol_def_set(from_mash_in_place_constructor *a2)
{
    if constexpr (0) {
    } else {
        THISCALL(0x005DD180, this, a2);
    }
}

void patrol_def_set::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_0, this);
}

void patrol_def_set::destruct_mashed_class()
{
    this->field_0.clear();
    this->field_0.destruct_mashed_class();
}
