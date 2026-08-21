#pragma once

#include <cstdint>

#include "mash.h"
#include "mash_virtual_base.h"
#include "mvector.h"
#include "mVectorBasic.h"
#include "string_hash.h"

struct combo_system_weapon;
struct combo_system_move;
struct from_mash_in_place_constructor;

struct combo_system_chain {
    struct telegraph_info : mash_virtual_base {
        int field_4;
        int field_8;

        //0x
        void _unmash(mash_info_struct *a1, void *a2);

        //virtual
        int get_mash_sizeof();
    };

    combo_system_chain(from_mash_in_place_constructor *a2);

    void initialize(mash::allocation_scope a2);

    mVector<combo_system_chain::telegraph_info> field_0;
    string_hash field_14;
    int field_18;
    mVectorBasic<int> field_1C;
    int field_2C;
    int field_30;
    int field_34;
    float field_38;
    float field_3C;
    float field_40;

    //0x004814E0
    void unmash(mash_info_struct *, void *);
};

struct combo_system {
    mVector<combo_system_move> field_0;
    mVector<combo_system_chain> field_14;
    mVector<combo_system_weapon> field_28;
    mVector<string_hash> field_3C;

    combo_system();

    //0x004896B0
    combo_system(from_mash_in_place_constructor *a2);

    //0x00489720
    void unmash(mash_info_struct *a1, void *a3);

    //0x00456300
    combo_system_weapon *get_weapon(int idx);

    //0x00446A50
    int get_num_weapons();
};

extern void combo_system_patch();
