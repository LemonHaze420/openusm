#include "camera_setup_entry.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"

VALIDATE_SIZE(camera_setup_entry, 0x38);

camera_setup_entry::camera_setup_entry(from_mash_in_place_constructor *a2)
    : field_0(a2), field_10(a2), field_20(a2)
{
}

void camera_setup_entry::unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_10, this);
    a1->unmash_class_in_place(this->field_20, this);
}
