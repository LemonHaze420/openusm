#pragma once

#include "param_block.h"
#include "resource_key.h"

namespace ai {
struct ai_core;
}

struct from_mash_in_place_constructor;

struct base_ai_data {
    resource_key field_0;
    ai::param_block field_8;
    ai::ai_core *field_14;

    //0x006D9AF0
    base_ai_data(from_mash_in_place_constructor *a2);

    //0x006BDB00
    void post_entity_mash();

    //0x006D7310
    void destruct_mashed_class();

    //0x006D7370
    void unmash(mash_info_struct *a1, void *a3);
};
