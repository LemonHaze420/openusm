#include "combo_system_weapon.h"

#include "common.h"
#include "mash_info_struct.h"
#include "trace.h"

VALIDATE_SIZE(combo_system_weapon, 0x24);

combo_system_weapon::combo_system_weapon(from_mash_in_place_constructor *a2) : field_0(a2), field_8(a2), field_10(a2), field_18(a2), field_1C(a2)
{
    this->initialize(mash::FROM_MASH);
}

void combo_system_weapon::initialize(mash::allocation_scope a2)
{
    if ( a2 == mash::ALLOCATED )
    {
        this->field_21 = true;
        this->field_20 = -1;
    }
}

void combo_system_weapon::unmash(mash_info_struct *, void *)
{}
