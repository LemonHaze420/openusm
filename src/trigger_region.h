#pragma once

#include "mash_virtual_base.h"

struct trigger_region : mash_virtual_base {
    //virtual
    void unmash(mash_info_struct *, void *);

    //virtual
    int get_mash_sizeof() const;
};
