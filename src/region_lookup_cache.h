#pragma once

#include "variable.h"

struct terrain;
struct region;

struct region_lookup_cache {
    //0x00514290
    static void init(terrain *a1);

    //0x00523D50
    static region *lookup_by_district_id(int a1);

    static void term();

    static inline Var<void *> district_ids{0x0095C8D8};

    static inline Var<void *> region_ptrs{0x0095C8DC};

    static inline Var<bool> initialized{0x0095C8E0};
};
