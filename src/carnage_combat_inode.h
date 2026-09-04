#pragma once

#include "combat_inode.h"

#include "float.hpp"

struct from_mash_in_place_constructor;

namespace ai {

struct carnage_combat_inode : combat_inode {
    int field_328[17];

    //0x0072F090
    carnage_combat_inode(from_mash_in_place_constructor *a2);

    //0x0071FDC0
    //virtual
    void frame_advance(Float a2);
};
}  // namespace ai
