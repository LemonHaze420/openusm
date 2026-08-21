#pragma once

#include "fetext.h"

struct mash_info_struct;

struct FEFloatingText : FEText {
    int field_68;
    int field_6C;
    int field_70;
    int field_74;
    int field_78;
    int field_7C;
    bool field_80;

    FEFloatingText();

    //virtual
    void _unmash(mash_info_struct *a1, void *a3);

    //virtual
    int _get_mash_sizeof();
};

extern void FEFloatingText_patch();
