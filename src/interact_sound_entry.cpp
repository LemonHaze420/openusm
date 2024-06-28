#include "interact_sound_entry.h"

#include "common.h"
#include "mash_info_struct.h"

VALIDATE_SIZE(interact_sound_entry, 0x14);

interact_sound_entry::interact_sound_entry(from_mash_in_place_constructor *a1) : field_0(a1)
{
    this->initialize(mash::FROM_MASH);
}

void interact_sound_entry::initialize(mash::allocation_scope a2)
{
    if ( a2 == mash::ALLOCATED )
    {
        this->field_0 = {0};
        this->field_4 = 0;
        this->field_8 = 0;
        this->field_C = 3.4028235e38;
        this->field_10 = true;
        this->field_11 = true;
    }
}

void interact_sound_entry::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_0, this);
}
