#pragma once

#include "enhanced_state.h"

#include "variable.h"
#include "vector3d.h"

namespace ai {
struct als_inode;

struct throw_state : enhanced_state {
    int field_30;
    float field_34[3];
    int field_40;
    int field_44;
    int field_48;
    int field_4C;
    bool field_50;
    vector3d field_54;
    float field_60;
    float field_64;
    float field_68;
    int field_6C;
    als_inode *field_70;
    int field_74;
    int field_78;

    throw_state();

    static const inline string_hash default_id{to_hash("THROW")};
};

}  // namespace ai
