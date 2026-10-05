#pragma once

#include "base_state.h"


namespace ai {
struct std_puppet_trans_state : base_state {
    std_puppet_trans_state();
    static void *native_vtable();

    //virtual
    void _unmash(mash_info_struct *a1, void *a2);

    //virtual
    int _get_virtual_type_enum() const
    {
        return 316;
    }

    //virtual
    int _get_mash_sizeof() const
    {
        return sizeof(*this);
    }
};
}  // namespace ai
