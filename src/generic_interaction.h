#pragma once

#include "interaction.h"

#include "interaction_type_enum.h"
#include "resource_key.h"
#include "string_hash.h"

struct ai_interaction_data;
struct generic_interaction : interaction {
    resource_key field_48;
    ai_interaction_data *field_50;

    //0x004D5DB0
    generic_interaction(const resource_key &resource_str, interaction_type_enum a3, string_hash a4);
    generic_interaction(from_mash_in_place_constructor *tag);
    void _unmash(mash_info_struct *info, void *context);
    static void *native_vtable();
};
