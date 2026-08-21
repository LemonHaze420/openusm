#pragma once

#include "color.h"
#include "fixedstring.h"

struct region_mash_info {
    fixedstring<8> field_0;
    color field_20;

    int field_30;
    int field_34;
    int field_38;
    int field_3C;

    //0x005C56D0
    region_mash_info();

    void *operator new(size_t);
};
