#pragma once

#include "mvector.h"
#include "mash_virtual_base.h"

namespace als
{
    struct transition_group_base : mash_virtual_base
    {
        transition_group_base();

        //virtual
        void _unmash(mash_info_struct *, void *);

        int get_mash_sizeof() const;
    };

}

extern void als_transition_group_base_patch();
