#pragma once

#include "hero_base_state.h"

#include "float.hpp"
#include "utility.h"

struct mash_info_struct;
struct string_hash;

namespace ai {

struct spidey_base_state : hero_base_state {
    spidey_base_state();

    //virtual
    void _unmash(mash_info_struct *a1, void *a2);

    //virtual
    int _get_virtual_type_enum() const {
        return 324;
    }

    int _get_mash_sizeof() const {
        return sizeof(*this);
    }

    //0x00488680
    /* virtual */ string_hash get_desired_state_id(Float a3) const /* override */;

    static inline void * g_vtbl[] {
        nullptr,
        func_address(&_unmash),
        nullptr,
        func_address(&_get_virtual_type_enum),
        nullptr,
        func_address(&_is_or_is_subclass_of),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        func_address(&_get_mash_sizeof)
    };
};

} // namespace ai

extern void spidey_base_state_patch();
