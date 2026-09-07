#include "tracking_panel.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"

VALIDATE_SIZE(tracking_panel, 0x7C);

tracking_panel::tracking_panel(from_mash_in_place_constructor *a2)
    : field_0(a2), field_10(a2), field_18(a2)
{
}

void tracking_panel::unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_10, this);
    a1->unmash_class_in_place(this->field_18, this);
}
