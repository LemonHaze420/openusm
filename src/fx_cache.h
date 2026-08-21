#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "float.hpp"
#include "mashable_vector.h"

struct generic_mash_header;
struct cached_special_effect;
struct generic_mash_data_ptrs;

struct fx_cache_ent {
    float field_0;
    int field_4;
    vhandle_type<entity> field_8;

    fx_cache_ent();

    void un_mash(generic_mash_header *a2, cached_special_effect *a3, void *a4, generic_mash_data_ptrs *a5);
};

struct fx_cache {
    int field_0;
    int field_4;
    mashable_vector<fx_cache_ent> field_8;
    fx_cache_ent *field_10;

    //0x00505750
    fx_cache();

    //0x004D3530
    ~fx_cache();

    //0x004D4FB0
    void frame_advance(Float a3);

    void un_mash(generic_mash_header *a2, cached_special_effect *a3, void *a4, generic_mash_data_ptrs *a5);
};

extern void fx_cache_patch();
