#pragma once

#include "base_state.h"

#include "utility.h"

namespace ai {
struct std_puppet_trans_state : base_state {
    std_puppet_trans_state();

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

    static inline void *g_vtbl[]{nullptr,
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
                                 func_address(&_get_mash_sizeof)};
};
}  // namespace ai
