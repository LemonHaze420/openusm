#pragma once

#include "entity_base_vhandle.h"
#include "info_node.h"
#include "mash.h"
#include "variable.h"

struct from_mash_in_place_constructor;

namespace ai {

struct std_fear_inode : info_node {
    bool field_1C;
    bool field_1D;
    bool field_1E;
    bool field_1F;
    bool field_20;
    bool field_21;
    bool field_22;
    bool field_23;
    bool field_24;
    bool field_25;
    bool field_26;
    bool field_27;
    int field_28;
    vector3d field_2C;
    float field_38;
    float field_3C;
    float field_40;
    float field_44;
    float field_48;
    float field_4C;
    float field_50;
    float field_54;
    float field_58;
    float field_5C;
    float field_60;
    bool field_64;
    entity_base_vhandle field_68;
    float field_6C;
    float field_70;
    float field_74;
    bool field_78;
    bool field_79;

    //0x006B0630
    std_fear_inode();

    //0x006B06B0
    std_fear_inode(from_mash_in_place_constructor *a2);

    //0x006A5090
    void initialize(mash::allocation_scope a2);

    //0x006A1810
    void add_to_list(_std::vector<ai::std_fear_inode *> **a2);

    //0x00691360
    void set_cowering_enabled(bool a2);

    static inline Var<_std::vector<std_fear_inode *> *> all_fear_inodes{0x0096BE30};

    static inline Var<_std::vector<std_fear_inode *> *> fleeing_fear_inodes{0x0096BE38};

    static inline Var<_std::vector<std_fear_inode *> *> cowering_fear_inodes{0x0096BE34};

    static inline const string_hash default_id{int(to_hash("std_fear_inode"))};
};

}  // namespace ai
