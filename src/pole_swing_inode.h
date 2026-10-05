#pragma once

#include "info_node.h"

#include "anchor_storage_class.h"

namespace ai {

struct pole_swing_inode : info_node {
    anchor_storage_class field_1C;
    int field_24;
    int field_28;

    static void *native_vtable();
    pole_swing_inode();
    explicit pole_swing_inode(from_mash_in_place_constructor *tag);

    bool is_eligible(string_hash) const;

    //0x0045D190
    bool can_go_to(string_hash arg0);

    static string_hash &default_id;
};

}  // namespace ai
