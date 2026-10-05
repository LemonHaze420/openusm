#pragma once

#include "hero_base_state.h"

#include "float.hpp"
#include "utility.h"

struct mash_info_struct;
struct string_hash;

namespace ai {

struct spidey_base_state : hero_base_state {
    spidey_base_state();
    explicit spidey_base_state(int mode);
    static void *native_vtable();

    //virtual
    void _unmash(mash_info_struct *a1, void *a2);

    //virtual
    int _get_virtual_type_enum() const
    {
        return 324;
    }

    int _get_mash_sizeof() const
    {
        return sizeof(*this);
    }

    //0x0044CFB0
    //virtual
    void _get_info_node_list(info_node_desc_list &a1);

    //0x00488680
    /* virtual */ string_hash get_desired_state_id(Float a3) const /* override */;
};

}  // namespace ai

extern void spidey_base_state_patch();
