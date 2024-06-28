#include "attach_interact_data.h"

#include "attach_node.h"
#include "common.h"
#include "mash_info_struct.h"
#include "trace.h"

VALIDATE_SIZE(attach_interact_data, 0x14);

attach_interact_data::attach_interact_data(from_mash_in_place_constructor *a2) : field_0(a2)
{}

void attach_interact_data::unmash(mash_info_struct *a1, void *)
{
    TRACE("attach_interact_data::unmash");

    a1->unmash_class_in_place(this->field_0, this);
}
