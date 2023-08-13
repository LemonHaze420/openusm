#pragma once

#include "string_hash.h"
#include "mvector.h"

struct mash_info_struct;

namespace als
{
    struct alter_conditions;

    struct post_layer_alter
    {
        int field_0;
        mVector<als::alter_conditions> field_4;
        string_hash field_18;

        //0x004ABD90
        void unmash(mash_info_struct *, void *);
    };
}
