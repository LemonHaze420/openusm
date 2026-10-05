#pragma once

#include "combat_inode.h"

namespace ai {

struct player_combat_inode : combat_inode {
    float field_328;
    float *field_32C;

    player_combat_inode();
    explicit player_combat_inode(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    int _get_virtual_type_enum() const { return 346; }
    int _get_mash_sizeof() const { return sizeof(*this); }

    //0x00467B90
    void _activate(ai_core *a2);
    void _frame_advance(Float delta);
};

}  // namespace ai
