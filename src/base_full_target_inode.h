#pragma once

#include "actor.h"
#include "entity_base_vhandle.h"
#include "info_node.h"

namespace ai {

struct base_full_target_inode : info_node {
    int *field_1C;
    int field_20;
    int field_24;
    int *field_28;
    int field_2C;
    int field_30;
    vhandle_type<actor> field_34;
    int field_38;
    int field_3C[18];

    //0x006A1990
    base_full_target_inode(from_mash_in_place_constructor *a2);

    //virtual
    vhandle_type<actor> quick_targeting();

    //virtual
    bool is_target_known();
};
}  // namespace ai
