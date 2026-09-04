#pragma once

#include "handheld_item.h"

struct grenade;

struct thrown_item : handheld_item {
    int field_114[138];
    int *field_33C;
    mString field_340;

    static inline Var<_std::vector<grenade *>> all_grenades{0x0095FF84};
};
