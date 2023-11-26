#pragma once

#include "interaction_type_enum.h"
#include "mvector.h"
#include "string_hash.h"
#include "trigger_region.h"

#include <cstdint>

struct from_mash_in_place_constructor;

struct interaction {
    std::intptr_t m_vtbl;
    mVector<trigger_region> field_4;

    int field_18[4];

    interaction_type_enum field_28;
    string_hash field_2C;
    int field_30;
    int field_34;
    float field_38;
    int field_3C;
    int field_40;
    bool field_44;
    bool field_45;

    //0x004D5A30
    interaction(from_mash_in_place_constructor *a2);

    //0x004D5D10
    interaction(interaction_type_enum a2, string_hash a3);

    //0x004BF3F0
    int get_approach();

    //0x004BF440
    void set_enabled(bool a2);

    //0x004BF2B0
    //virtual
    int get_virtual_type_enum();
};
