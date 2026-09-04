#include "attach_node.h"

#include "common.h"
#include "mash_info_struct.h"
#include "trace.h"

VALIDATE_SIZE(attach_node, 0x28);

attach_node::attach_node(from_mash_in_place_constructor *a2) : field_0(a2), field_10(a2)
{
    this->initialize(mash::FROM_MASH);
}

void attach_node::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
        this->field_24 = -1;
    }
}

void attach_node::unmash(mash_info_struct *a1, void *)
{
    TRACE("attach_node::unmash");

    a1->unmash_class_in_place(this->field_0, this);

    a1->unmash_class_in_place(this->field_10, this);
}
